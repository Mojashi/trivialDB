#pragma once

#include <string>
#include <list>
#include <shared_mutex>

#include "types.hpp"

const extern TimeStamp start_ts;
const extern TimeStamp minf;
const extern TimeStamp pinf;
const extern TransactionId none;
const extern TransactionId superTx;

#include "utils.hpp"
#include "table.hpp"

using std::string;

template <typename V>
class Version{
    const TimeStamp created_ts_;
    TransactionPtr overWriter_;
    volatile TimeStamp pstamp_ = minf,sstamp_ = pinf;
    volatile TimeStamp overWriterCstamp_;

    std::list<TransactionPtr> readers_;
    volatile TransactionId readerLock = none;
    // std::shared_mutex rmtx;

    const bool deleted_;
    const V val_;

    const VerPtr<V> prev_;
    
public:
    Version(V val_, VerPtr<V> prev_,TimeStamp created_ts_);
    Version(bool deleted_,VerPtr<V> prev_, TimeStamp created_ts_);
    Version(const Version& v);

    void updPstamp(TimeStamp ts);
    void updSstamp(TimeStamp ts);

    void setOverWriter(TransactionPtr tx);
    TransactionPtr overWriter();

    std::list<TransactionPtr> readers(TransactionId id);
    void addReader(TransactionPtr tx);

    TimeStamp overWriterCstamp();
    //TimeStamp pstamp();
    TimeStamp sstamp();
    TimeStamp created_ts();
    V val();
    VerPtr<V> prev();
    bool deleted();
};

template class Version<string>;


template<typename V>
class Record{
    VerPtr<V> latest_;
    volatile TransactionId writerLock = none;

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