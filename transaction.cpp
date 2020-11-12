#include "transaction.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdexcept>
#include "utils.hpp"
#include <algorithm>

using std::endl;

Transaction::Transaction(Table* table, long long timestamp,std::istream& is, std::ostream& os) : is(is), os(os), table(table), timestamp(timestamp) {}

void Transaction::begin() {
	os << "TransactionID:" << timestamp << endl;
	while (!commited && !aborted) {
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
		} catch (OperationException& e) {
			os << e.what() << endl;
		} catch (std::invalid_argument& e) {
			os << e.what() << endl;
		}
	}
}

string Transaction::get(const string& key) {
	if (!validateKey(key)) throw InvalidKeyError();

	if (!exist(key)) throw RecordDoesNotExistError();
	string ret;
	if (readSet.count(key) > 0)
		ret = readSet[key];
	else if (writeSet.count(key) > 0)
		ret = writeSet[key];
	else
		ret = table->get(key);
	return readSet[key] = ret;
}

void Transaction::remove(const string& key, bool deleteIfNotExist) {
	if (!validateKey(key)) throw InvalidKeyError();

	if (!deleteIfNotExist && !exist(key)) throw RecordDoesNotExistError();
	deleteSet.insert(key);
	writeSet.erase(key);
	readSet.erase(key);
}
void Transaction::update(const string& key, const string& value,
						 bool insertIfNotExist) {
	if (!validateKey(key)) throw InvalidKeyError();

	remove(key, insertIfNotExist);
	insert(key, value);
}

void Transaction::insert(const string& key, const string& value,
						 bool updateIfExist) {
	if (!validateKey(key)) throw InvalidKeyError();

	if (!updateIfExist && exist(key)) throw RecordExistError();
	deleteSet.erase(key);
	writeSet[key] = value;
	readSet[key] = value;
}

bool Transaction::exist(const string& key) {
	if (!validateKey(key)) throw InvalidKeyError();

	return !(deleteSet.count(key) > 0) &&
		   (writeSet.count(key) > 0 || table->exist(key));
}

void Transaction::writeRedoLog(const string& fname) {
	string redolog =
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
			os << "an error occurred while wfriting to file" << endl;
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

bool Transaction::commit() {
	writeRedoLog(redoLogFile);
	table->applyRedoLog(writeSet, deleteSet);
	commited = true;
}

bool Transaction::abort() { aborted = true; }