#pragma once
#include <map>
#include <string>
#include <set>
#include "cnf.hpp"
#include "table.hpp"
#include "record.hpp"
#include <iostream>


extern const bool BENCH;

using std::map;
using std::string;
using std::set;
class Table;
class Transaction : public std::enable_shared_from_this<Transaction> {
    TransactionId id;
    bool commited = false, aborted = false;
    map<string, std::shared_ptr<Record<string>>> rLocks, wLocks;
public:
    enum Status{
        INFLIGHT,
        COMMITTING,
        COMMITTED,
        ABORTED,
    };

    volatile Status status_ = INFLIGHT;
private:
    TransactionId id;

    map<string,RecordPtr> wLocks;
    map<string,string> writeSet, readSet; //readSet includes writeSet
    set<string> deleteSet;
    Table* table;
    std::ostream& os;
    std::istream& is;

    void writeRedoLog(const string& fname);

    void getWriteLock(const string& key);
    void getReadLock(const string& key);
    void applyToTable();

public:
    Transaction(Table* table, TransactionId id, std::istream& is = std::cin, std::ostream& os = std::cout);
    bool commit();
    bool abort();
    void releaseRLocks();
    void releaseWLocks();

    void begin();

    string get(const string& key);

    void remove(const string& key, bool deleteIfNotExist=false);
    void update(const string& key, const string& value, bool insertIfNotExist=false);
    void insert(const string& key, const string& value, bool updateIfExist=false);  
    bool exist(const string& key);

    void freeMem();
};

