#pragma once

#include <string>
#include <list>
#include <shared_mutex>
#include "types.hpp"
#include "utils.hpp"

using std::string;

const extern TimeStamp start_ts;
const extern TimeStamp minf;
const extern TimeStamp pinf;
const extern TransactionId none;
const extern TransactionId superTx;

template <typename V>
class Record{
    std::list<TransactionId> readerIds;
    volatile int readerCou = 0;
    volatile bool phantomRecord_;

    volatile TransactionId oldestReaderCache = none;
    volatile unsigned long long int listVer = -1, cacheVer = -1;

    volatile TransactionId writerId = none;
    std::shared_mutex mtx;

    V val_;
    
public:
    Record(V val_);
    Record(bool phantomRecord_);
    // Record(const Record<V> r);
    V val(TransactionId id);
    void set(TransactionId id, V new_val);

    void setPhantomRecord(TransactionId id);
    bool phantomRecord(TransactionId id);
    bool RLock(TransactionId id);
    void RUnLock(TransactionId id);

    TransactionId getOldestTransId();

    bool WLock(TransactionId id);
    void WUnLock(TransactionId id);

    bool Upgrade(TransactionId id);
};

template class Record<string>;


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

class UnknownReaderException : LockingException {
    public:
    UnknownReaderException() : LockingException("unknown reader tried to read value"){}
};
