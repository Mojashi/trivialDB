#include "record.hpp"
#include <algorithm>

const TimeStamp start_ts = 2;
const TimeStamp minf = 0;
const TimeStamp pinf = std::numeric_limits<TimeStamp>::max();
const TransactionId none = 0;
const TransactionId superTx = 1;

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
	if(writerId != id) throw UnknownWriterException();
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
	if(writerId != id) throw UnknownWriterException();
	phantomRecord_ = true;
}

template<typename V>
bool Record<V>::RLock(TransactionId id) {
	while (writerId > id) {
		if (writerId == none) {
			std::lock_guard<std::shared_mutex> lock(mtx);
			if (writerId == none) {
				readerIds.push_back(id);
				listVer++;
				return true;
			}
		}
	}
	return false;  // die
}

template<typename V>
void Record<V>::RUnLock(TransactionId id) {
	std::shared_lock<std::shared_mutex> lock;
    
	auto itr = find(readerIds.begin(), readerIds.end(), id);
	if (itr == readerIds.end()) return;
	readerIds.erase(itr);
}

template<typename V>
TransactionId Record<V>::getOldestTransId() {
	if (cacheVer == listVer) return std::min(writerId, oldestReaderCache);

	std::shared_lock<std::shared_mutex> lock(mtx);
	TransactionId ret = none;
	for (auto e : readerIds) {
		ret = std::min(ret, e);
	}
	cacheVer = listVer;
	return oldestReaderCache = ret;
}

template<typename V>
bool Record<V>::WLock(TransactionId id) {
	while (getOldestTransId() > id) {
		if (writerId == none && readerIds.size() == 0) {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (writerId == none && readerIds.size() == 0) {
				writerId = id;
				return true;
			}
		}
	}
	return false;  // die
}

template<typename V>
void Record<V>::WUnLock(TransactionId id) {
	std::lock_guard<std::shared_mutex> lock(mtx);
	if (id == writerId) writerId = none;
}

template<typename V>
bool Record<V>::Upgrade(TransactionId id) {
	{
		std::lock_guard<std::shared_mutex> lock(mtx);
		if (find(readerIds.begin(), readerIds.end(), id) == readerIds.end())
			return false;  // nazo
	}

	while (getOldestTransId() >= id) {
		if (writerId == none && readerIds.size() == 1) {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (writerId == none && readerIds.size() == 1) {
				RUnLock(id);
				writerId = id;
				return true;
			}
		}
	}
	return false;  // die
}