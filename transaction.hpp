#pragma once
#include <map>
#include <string>
#include <set>
#include "types.hpp"
#include "cnf.hpp"
#include "table.hpp"
#include "record.hpp"
#include <iostream>

using std::map;
using std::string;
using std::set;
class Table;
class Transaction : public std::enable_shared_from_this<Transaction> {
    TransactionId id;
    enum Status{
        INFLIGHT,
        COMMITTING,
        COMMITTED,
        ABORTED,
    } status_;
    
    TimeStamp cstamp_ = minf,pstamp_ = minf,sstamp_ = pinf;

    map<string,RecordPtr> wLocks;
    map<string, VerPtr<string>> readVs;
    map<string,string> writeSet, readSet; //readSet includes writeSet
    set<string> deleteSet;
    Table* table;
    std::ostream& os;
    std::istream& is;

    void writeRedoLog();
    void getWriteLock(const string& key);
    void applyToTable();
    void fetch(const string& key);
    bool ssnCheckTransaction();

public:
    Transaction(Table* table, TransactionId id, std::istream& is = std::cin, std::ostream& os = std::cout);
    void commit();
    void abort();
    void releaseWLocks();

    Status status();
    TimeStamp cstamp();
    TimeStamp sstamp();
    TimeStamp pstamp();

    void begin();

    string get(const string& key);

    void remove(const string& key, bool deleteIfNotExist=false);
    void update(const string& key, const string& value, bool insertIfNotExist=false);
    void insert(const string& key, const string& value, bool updateIfExist=false);  
    bool exist(const string& key);
};

