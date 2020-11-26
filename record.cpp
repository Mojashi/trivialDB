#include "record.hpp"
#include <algorithm>
#include <limits>
#include <atomic>

const TimeStamp start_ts = 2;
const TimeStamp minf = 0;
const TimeStamp pinf = std::numeric_limits<TimeStamp>::max();
const TransactionId none = 0;
const TransactionId superTx = 1;

template<typename V>
Version<V>::Version(V val_, VerPtr<V> prev_,TimeStamp created_ts_):val_(val_), prev_(prev_), created_ts_(created_ts_), pstamp_(created_ts_), deleted_(false){
	// std::cout << created_ts_ << std::endl;
}

template<typename V>
Version<V>::Version(bool deleted_,VerPtr<V> prev_,TimeStamp created_ts_):deleted_(deleted_), created_ts_(created_ts_), pstamp_(created_ts_), prev_(prev_){}

template<typename V>
Version<V>::Version(const Version& v):val_(v.val_), prev_(v.prev_), created_ts_(v.created_ts_), pstamp_(v.pstamp_), deleted_(v.deleted_), sstamp_(v.sstamp_) {}

template<typename V>
V Version<V>::val(){
	return val_;
}

template<typename V>
VerPtr<V> Version<V>::prev(){
	return prev_;
}

template<typename V>
bool Version<V>::deleted(){
	return deleted_;
}

template<typename V>
TimeStamp Version<V>::created_ts(){
	return created_ts_;
}

template<typename V>
TimeStamp Version<V>::sstamp(){
	return sstamp_;
}

// template<typename V>
// TimeStamp Version<V>::pstamp(){
// 	return pstamp_;
// }

template<typename V>
void Version<V>::updPstamp(TimeStamp ts){
	TimeStamp buf = ts;
	while(pstamp_ < buf){
		if(__sync_bool_compare_and_swap(&pstamp_, buf, ts)) break;
		buf = ts;
	}
}

template<typename V>
void Version<V>::updSstamp(TimeStamp ts){
	sstamp_ = ts;
}

template<typename V>
void Version<V>::addReader(TransactionPtr tx){
	std::lock_guard<std::shared_mutex> lock(rmtx);
	readers_.push_back(tx);
}

template<typename V>
std::list<TransactionPtr> Version<V>::readers(){
	std::shared_lock<std::shared_mutex> lock(rmtx);
	return readers_;
}

template<typename V>
TimeStamp Version<V>::overWriterCstamp(){
	return overWriterCstamp_;
}

template<typename V>
void Version<V>::setOverWriter(TransactionPtr tx){
	overWriter_ = tx;
}
template<typename V>
TransactionPtr Version<V>::overWriter(){
	return overWriter_;
}

template<typename V>
Record<V>::Record(){
	latest_ = (VerPtr<V>)(new Version<V>(true, NULL, minf));
}

template<typename V>
Record<V>::Record(V initVal){
	latest_ = (VerPtr<V>)(new Version<V>(initVal, NULL, minf));
}

template<typename V>
VerPtr<V> Record<V>::latest(){return latest_;}

template<typename V>
VerPtr<V> Record<V>::findVersion(TimeStamp ts){
	VerPtr<V> cur = latest_;

	while(cur != NULL){
		if(cur->created_ts() < ts) return cur;
		cur = cur->prev();
	}
	return NULL;
}


template<typename V>
bool Record<V>::WLock(TransactionId id){
	while(1){
		if(writerLock == none){
			if(__sync_bool_compare_and_swap(&writerLock, none, id))
				return true;
		}
	}
}

template<typename V>
bool Record<V>::WUnLock(TransactionId id){
	if(id != writerLock) false;
	writerLock = none;
	return true;
}

template<typename V>
VerPtr<V> Record<V>::addVersion(const Version<V>& v, TransactionId id){
	if(writerLock != id) throw UnknownWriterException();
	std::atomic_store(&latest_, (VerPtr<V>)(new Version<V>(v)));
	return latest_;
}

template<typename V>
VerPtr<V> Record<V>::remove(TransactionId id, TimeStamp ts){
	if(writerLock != id) throw UnknownWriterException();

	Version<V> v(true, NULL, ts);
	return addVersion(v, id);
}

template<typename V>
VerPtr<V> Record<V>::update(V val, TransactionId id, TimeStamp ts){
	if(writerLock != id) throw UnknownWriterException();
	Version<V> v(val, latest(), ts);
	return addVersion(v, id);
}

template<typename V>
Record<V>::~Record<V>(){
	// WLock(superTx);
	// VerPtr<V> cur = latest_;
	// while(cur != NULL){
	// 	VerPtr<V> nex = cur->prev();
	// 	delete cur;
	// }
	// WUnLock(superTx);
}
