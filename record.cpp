#include "record.hpp"
#include <algorithm>

const TimeStamp start_ts = 2;
const TimeStamp minf = 0;
const TransactionId none = 0;
const TransactionId superTx = 1;

template<typename V>
Version<V>::Version(V val_, VerPtr<V> prev_,TimeStamp created_ts):val_(val_), prev_(prev_), created_ts_(created_ts_), deleted_(false){}

template<typename V>
Version<V>::Version(bool deleted_,VerPtr<V> prev_,TimeStamp created_ts):deleted_(deleted_), created_ts_(created_ts_), val_(""), prev_(prev_){}

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

	return latest_ = (VerPtr<V>)(new Version<V>(v)); //ここでsharedptrのコピーがatomicに行われないので生ポインタを使っている atomic_shared_ptrなるものがあるらしい
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
	WLock(superTx);
	VerPtr<V> cur = latest_;
	while(cur != NULL){
		VerPtr<V> nex = cur->prev();
		delete cur;
	}
	WUnLock(superTx);
}
