#pragma once

#include <string>
#include <list>
#include <shared_mutex>
#include "utils.hpp"

using std::string;
using TimeStamp = unsigned long long int;
using TransactionId = TimeStamp;

const extern TimeStamp start_ts;
const extern TimeStamp minf;
const extern TransactionId none;
const extern TransactionId superTx ;

template<typename V>
class Version;

template<typename V>
using VerPtr = Version<V>*;

template <typename V>
class Version{
    const TimeStamp created_ts_;
    const bool deleted_;
    const V val_;

    const VerPtr<V> prev_;
    
public:
    Version(V val_, VerPtr<V> prev_,TimeStamp created_ts_);
    Version(bool deleted_,VerPtr<V> prev_, TimeStamp created_ts_);

    TimeStamp created_ts();
    V val();
    VerPtr<V> prev();
    bool deleted();
};

template class Version<string>;


template<typename V>
class Record{
    VerPtr<V> latest_;
    TransactionId writerLock;

public:
    Record();
    Record(V initVal);

    VerPtr<V> findVersion(TimeStamp ts);
    VerPtr<V> latest();
    VerPtr<V> addVersion(const Version<V>& v, TransactionId id);

    VerPtr<V> remove(TransactionId id, TimeStamp ts);
    VerPtr<V> update(V val, TransactionId id, TimeStamp ts);

    bool WLock(TransactionId id);
    bool WUnLock(TransactionId id);

    ~Record();
};


class LockingException : std::exception{
	const char* s;

   public:
	LockingException(const char* s) : s(s) {}
	const char* what() const noexcept { return s; }
};

class UnknownWriterException : LockingException {
    public:
    UnknownWriterException() : LockingException("unknown writer tried to write value"){}
};

template class Record<string>;