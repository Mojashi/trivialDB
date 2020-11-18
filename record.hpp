#pragma once

#include <string>
#include <list>
#include <shared_mutex>
#include "utils.hpp"

using std::string;
using TransactionId = unsigned int;

extern const TransactionId none;

template <typename V>
class Record{
    std::list<TransactionId> readerIds;
    bool phantomRecord_;

    TransactionId oldestReaderCache = none;
    unsigned long long int listVer = -1, cacheVer = -1;

    TransactionId writerId = none;
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
