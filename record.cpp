#include "record.hpp"
#include <algorithm>

const TimeStamp start_ts = 2;
const TimeStamp minf = 0;
const TimeStamp pinf = std::numeric_limits<TimeStamp>::max();
const TransactionId none = 0;
const TransactionId superTx = 1;
const TransactionId readerTx = 1;

template<typename V>
Record<V>::Record(V val_):val_(val_){}
template<typename V>
Record<V>::Record(bool phantomRecord_):phantomRecord_(phantomRecord_){}

template<typename V>
V Record<V>::val(TransactionId id){
	// if(id not in readerIds) throw UnknoenReaderException();
	// if(phantomRecord()) throw 
    return val_;
}

template<typename V>
void Record<V>::set(TransactionId id,V new_val){
	if(writerId != id) 
		throw UnknownWriterException();
	phantomRecord_ = false;
	val_ = new_val;
}

template<typename V>
bool Record<V>::phantomRecord(TransactionId id){
	// if(id not in readerIds) throw UnknoenReaderException();
    return phantomRecord_;
}
template<typename V>
void Record<V>::setPhantomRecord(TransactionId id){
	if(writerId != id)
		throw UnknownWriterException();
	phantomRecord_ = true;
}

template<typename V>
bool Record<V>::RLock(TransactionId id) {
	while (writerId == none || writerId > id) {
		if (writerId == none) {
			if(__sync_bool_compare_and_swap(&writerId,none,readerTx)){
				std::lock_guard<std::shared_mutex> lock(mtx);
				readerIds.push_back(id);
				__sync_fetch_and_add(&readerCou, 1);
				__sync_fetch_and_add(&listVer, 1);
				writerId = none;
				return true;
			}
		}
	}
	return false;  // die
}

template<typename V>
void Record<V>::RUnLock(TransactionId id) {
	std::lock_guard<std::shared_mutex> lock(mtx);
    
	auto itr = find(readerIds.begin(), readerIds.end(), id);
	if (itr == readerIds.end()) return;
	readerIds.erase(itr);
	readerCou--;
}

template<typename V>
TransactionId Record<V>::getOldestTransId() {
	// if (cacheVer == listVer) return std::min(writerId == none ? oldestReaderCache:writerId, oldestReaderCache);
	std::shared_lock<std::shared_mutex> lock(mtx);
	if(readerCou == 0) return writerId;

	TransactionId bw = writerId;
	TransactionId ret = bw == none ? readerIds.front() : bw;;
	for (auto e : readerIds) {
		ret = std::min(ret, e);
	}
	// cacheVer = listVer;
	return oldestReaderCache = ret;
}

template<typename V>
bool Record<V>::WLock(TransactionId id) {
	auto b = getOldestTransId();
	while (b == none || b > id) {
		if (writerId == none && readerCou == 0) {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (readerCou == 0) {
				if(__sync_bool_compare_and_swap(&writerId,none,id)){
					return true;
				}
			}
		}
		b = getOldestTransId();
	}
	return false;  // die
}

template<typename V>
void Record<V>::WUnLock(TransactionId id) {
	if (id != writerId) 
		throw UnknownWriterException();
	writerId = none;
}

template<typename V>
bool Record<V>::Upgrade(TransactionId id) {
	{
		std::shared_lock<std::shared_mutex> lock(mtx);
		if (find(readerIds.begin(), readerIds.end(), id) == readerIds.end())
			return false;  // nazo
	}

	auto b = getOldestTransId();
	while (b == none || b > id) {
		if (writerId == none && readerCou == 1) {
			std::lock_guard<std::shared_mutex> lock(mtx);
			if (readerCou == 1) {
				if(__sync_bool_compare_and_swap(&writerId,none,id)){
					readerIds.erase(find(readerIds.begin(), readerIds.end(), id));
					readerCou--;
					return true;
				}
			}
		}
		b = getOldestTransId();
	}
	return false;  // die
}