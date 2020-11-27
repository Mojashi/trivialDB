#include "transaction.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdexcept>
#include "utils.hpp"
#include <algorithm>
#include <cassert>
#include "table.hpp"

using std::endl;

Transaction::Transaction(Table* table, TransactionId id,std::istream& is, std::ostream& os) : is(is), os(os), table(table), id(id) {}

void Transaction::begin() {
	os << "TransactionID:" << id << endl;
	while (status_ == INFLIGHT) {
		os << "Transaction > ";
		string s;
		getline(is, s);

		replace(s.begin(),s.end(), '\n', ' ');
		replace(s.begin(),s.end(), '\r', ' ');

		vector<string> args = split(s, ' ');
		if (args.size() == 0) continue;

		try {
			if (args[0] == "update") {
				if (args.size() < 3)
					throw std::invalid_argument("Not enough arguments");
				update(args[1], args[2]);
				os << "OK" << endl;
			} else if (args[0] == "insert") {
				if (args.size() < 3)
					throw std::invalid_argument("Not enough arguments");
				insert(args[1], args[2]);
				os << "OK" << endl;
			} else if (args[0] == "delete") {
				if (args.size() < 2)
					throw std::invalid_argument("Not enough arguments");
				remove(args[1]);
				os << "OK" << endl;
			} else if (args[0] == "read") {
				if (args.size() < 2)
					throw std::invalid_argument("Not enough arguments");
				os << get(args[1]) << endl;
			} else if (args[0] == "commit") {
				commit();
				os << "successfully commited" << endl;
			} else if (args[0] == "abort") {
				abort();
				os << "successfully aborted" << endl;
			} else if (args[0] == "help") {
				os << "update insert delete read commit abort help" << endl;
			} else {
				os << "unknown operation" << endl;
			}
		} catch (CouldntLockResourceError& e) {
			os << e.what() << endl;
			abort();
			os << "this transaction has aborted" << endl;
		} catch (SSNCheckFailedError& e) {
			os << e.what() << endl;
			abort();
			os << "this transaction has aborted" << endl;
		} catch (OperationException& e) {
			os << e.what() << endl;
		} catch (std::invalid_argument& e) {
			os << e.what() << endl;
		}
	}
	freeMem();
}

TransactionId Transaction::getId(){
	return id;
}

void Transaction::fetch(const string& key){
	if(readSet.count(key) || deleteSet.count(key)) return;
	auto r = table->get(key);
	assert(r);
	auto v = r->latest();
	readVs[key] = v;
	v->addReader(shared_from_this());
	// pstamp = std::max(pstamp, v->created_ts());

	if(v->deleted())
		deleteSet.insert(key);
	else
		readSet[key] = r->latest()->val();
}

string Transaction::get(const string& key) {
	if (!validateKey(key)) throw InvalidKeyError();

	fetch(key);
	if (readSet.count(key) > 0)
		return readSet[key];
	if (deleteSet.count(key)) throw RecordDoesNotExistError();
	assert(true);
	return "";
}

void Transaction::getWriteLock(const string& key){
	if(wLocks.count(key) > 0) 
		return; 

	RecordPtr record = table->get(key);
	bool suc = record.get()->WLock(id);
	if(suc){
		wLocks[key] = record;
		auto v = record->latest();
		v->setOverWriter(shared_from_this());

		if(v->deleted()){
			deleteSet.insert(key); //あんまよくない
		}
		else {
			readSet[key] = v->val();
		}
	}else {
		throw CouldntLockResourceError();
	}
}

void Transaction::remove(const string& key, bool deleteIfNotExist) {
	if (!validateKey(key)) throw InvalidKeyError();

	getWriteLock(key);

	if (!deleteIfNotExist && !exist(key)) throw RecordDoesNotExistError();
	deleteSet.insert(key);
	writeSet.erase(key);
	readSet.erase(key);
}

void Transaction::update(const string& key, const string& value,
						 bool insertIfNotExist) {
	if (!validateKey(key)) throw InvalidKeyError();

	getWriteLock(key);

	remove(key, insertIfNotExist);
	insert(key, value);
}

void Transaction::insert(const string& key, const string& value,
						 bool updateIfExist) {
	if (!validateKey(key)) throw InvalidKeyError();

	getWriteLock(key);

	if (!updateIfExist && exist(key)) throw RecordExistError();
	
	deleteSet.erase(key);
	writeSet[key] = value;
	readSet[key] = value;
}

bool Transaction::exist(const string& key) {
	if (!validateKey(key)) throw InvalidKeyError();
	fetch(key);
	return deleteSet.count(key) == 0;
}

void Transaction::writeRedoLog() {
	string fname = string(redoLogDir) +"/"+ std::to_string(cstamp_) + ".log";

	string redolog =
		"$" + std::to_string(cstamp_) + "\n"+
		"$" + std::to_string(deleteSet.size() + writeSet.size()) + "\n";

	for (auto w : deleteSet) {
		redolog += "$6\ndelete\n$" + std::to_string(w.size()) + "\n" + w + "\n";
	}
	for (auto w : writeSet) {
		redolog += "$3\nset\n$" + std::to_string(w.first.size()) + "\n" +
				   w.first + "\n$" + std::to_string(w.second.size()) + "\n" +
				   w.second + "\n";
	}

	unsigned int sum = checksum(redolog.c_str(), redolog.size()),
				 sz = redolog.size();
	if (sz > MAX_TRANSACTION_SIZE) 
        throw TooLargeTransactionError();

	redolog = "$" + std::to_string(sum) + "\n$" + std::to_string(sz) + "\n" + redolog;

	int fd = open(fname.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1) 
		throw std::runtime_error("an error occurred while opening file");
	
	// write中とかにエラー起きると永続化されてるのかされてないのかわからないので一回落として再起動時のcrash
	// recoveryに託す
	int wrote = 0, wsz = 0;
    os << "writing to redo.log" << endl;
	while (wrote < redolog.size()) {
		if ((wsz = write(fd, redolog.c_str() + wrote,
						 redolog.size() - wrote)) == -1) {
			os << "an error occurred while writing to file" << endl;
			close(fd);
			exit(1);
		}
		wrote += wsz;
	}
    os << "OK!" << endl;
	if (fsync(fd) == -1) {
		os << "an error occurred while syncing file" << endl;
		exit(1);
	}
	if (close(fd) == -1) {
		os << "an error occurred while closing file" << endl;
		exit(1);
	}
}

void Transaction::commit() {
	status_ = COMMITTING;
	sstamp_ = cstamp_ = table->getTimeStamp();

	ssnCheckTransaction();

	if(!BENCH)
		writeRedoLog();

	applyToTable();
	status_ = COMMITTED;
	releaseWLocks();
}

void Transaction::abort() {
	status_ = ABORTED;	
	releaseWLocks();
}

void Transaction::releaseWLocks(){
	for(auto& wlock : wLocks){
		wlock.second->WUnLock(id);
	}
}

void Transaction::applyToTable(){
	for(auto& w : wLocks){
		RecordPtr record = w.second;
		auto v = record->latest();
		
		v->updSstamp(sstamp_);
	}

	for(auto& w : writeSet){
		RecordPtr record = wLocks[w.first];
		record->update(w.second, id, cstamp_);
	}
	for(auto& d : deleteSet){
		if(wLocks.count(d) == 0) continue;
		RecordPtr record = wLocks[d];
		record->remove(id, cstamp_);
	}
}

bool Transaction::ssnCheckTransaction(){
	os << "lets check" << cstamp_ <<endl;

	for(auto& v : readVs){
		pstamp_ = std::max((TimeStamp)pstamp_, v.second->created_ts()); //w-r(自分) update eta(T)

		if(v.second->sstamp() < sstamp_){ // committed or committing
			TransactionPtr ptr = v.second->overWriter();
			if(!ptr){ //committed
				if(v.second->overWriterCstamp() < cstamp_){
					sstamp_ = std::min((TimeStamp)sstamp_, v.second->sstamp()); // r(じぶん)-w update pi(T)
				}
			}else {
				if(ptr->status() != INFLIGHT){ // ptr->cstamp() != minf だと間違い
					while(ptr->cstamp() == minf);
					if(ptr->cstamp() >= cstamp_) continue;

					while(ptr->status() == COMMITTING);
					if(ptr->status() == COMMITTED)
						sstamp_ = std::min((TimeStamp)sstamp_, ptr->sstamp()); //  r(じぶん)-w update pi(T)				
				}
			}
		}
	}

	//os << "wlock check" << cstamp_ <<endl;

	for (auto& w : wLocks) {
		RecordPtr record = w.second;
		auto v = record->latest();
		auto readers = v->readers(id);
	//os << "readers check" << cstamp_ <<endl;
		pstamp_ = std::max((TimeStamp)pstamp_, v->created_ts()); // w(だれか)-w(じぶん) update eta(T)

		for(auto& r : readers){
	//		os << "a" << cstamp_<<"," <<r->cstamp() <<endl;
			if(!r) continue;
			if(r->status() == INFLIGHT || r->status() == ABORTED) continue;
	//os << "b" << cstamp_ <<endl;
			while(r->cstamp() == minf){}
	//os << "v" << cstamp_ <<endl;
			if(r->cstamp() >= cstamp_) continue;
	//os << "d" << cstamp_ <<endl;
			while(r->status() == COMMITTING){
			// os << "w" << cstamp_<<"," <<r->cstamp() <<endl;
			}
	//os << "r" << cstamp_ <<endl;
			if(r->status() == COMMITTED)
				pstamp_ = std::max((TimeStamp)pstamp_, r->cstamp()); // r(だれか)-w(じぶん) update eta(T)
		}
	}

	//os << "cts:" << cstamp_ << " pi:" << sstamp_ << " eta:" << pstamp_ << std::endl;
	if(sstamp_ <= pstamp_)
		throw SSNCheckFailedError();
	
	return true;
}

Transaction::Status Transaction::status(){
	return status_;
}

TimeStamp Transaction::cstamp(){
	return cstamp_;
}
TimeStamp Transaction::sstamp(){
	return sstamp_;
}

void Transaction::freeMem(){
	wLocks.clear();
	readVs.clear();
	writeSet.clear();
	readSet.clear();
	deleteSet.clear();
}