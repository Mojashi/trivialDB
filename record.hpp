#pragma once

#include <string>
#include <shared_mutex>
#include <list>
#include "utils.hpp"

using std::string;
using TransactionId = unsigned int;

extern const TransactionId none;

template <typename V>
class Record{
    std::list<TransactionId> readerIds;

    TransactionId oldestReaderCache = none;
    unsigned long long int listVer = -1, cacheVer = -1;

    TransactionId writerId = none;
    std::shared_mutex mtx;

    V val_;
    
public:
    Record(V val_);
    // Record(const Record<V> r);
    V val();

    bool RLock(TransactionId id);
    void RUnLock(TransactionId id);

    TransactionId getOldestTransId();

    bool WLock(TransactionId id);
    void WUnLock(TransactionId id);

    bool Upgrade(TransactionId id);
};

template class Record<string>;