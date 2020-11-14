#pragma once
#include <string>

using std::string;

typedef usigned int TransactionId;

const TransactionId none;

template<typename V>
class Record{
    V val;

    unsigned int readerCount = 0;
    TransactionId readerId,writerId;
public:
    V get();

    void RLock(TransactionId id){
        while((writerId < id){
            bool suc = __sync_bool_compare_and_swap(&readerId, none, id);
            if(suc){
                __sync_fetch_and_add(&readerCount, 1);
                break;
            }
        }
    }

    void RUnLock(TransactionId id){
        if(readerCount > 0)
            __sync_fetch_and_sub(&readerCount, 1);
    }

    void WLock(TransactionId id){
        while(writerId < id && readerId < id){
            if(writerId == none){
                bool suc = __sync_bool_compare_and_swap(&writerId, none, id);
                if(suc){
                    __sync_fetch_and_add(&readerCount, 1);
                    break;
                }
            }
        }
    }
    void WUnLock(){

    }

};