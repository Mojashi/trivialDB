#include "record.hpp"
#include <algorithm>

const TransactionId none = UINT32_MAX;

template<typename V>
Record<V>::Record(V val_):val_(val_){}

template<typename V>
V Record<V>::val(){
    return val_;
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

	while (getOldestTransId() > id) {
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