#pragma once
#include <map>
#include <string>
#include <set>
#include "cnf.hpp"
#include "table.hpp"
#include "record.hpp"
#include <iostream>

using std::map;
using std::string;
using std::set;
class Table;
class Transaction{
    TransactionId id;
    bool commited = false, aborted = false;
    TimeStamp cstamp = minf;

    map<string,RecordPtr> wLocks;
    map<string,string> writeSet, readSet; //readSet includes writeSet
    set<string> deleteSet;
    Table* table;
    std::ostream& os;
    std::istream& is;

    void writeRedoLog(const string& fname);
    void getWriteLock(const string& key);
    void applyToTable();
    void fetch(const string& key);

public:
    Transaction(Table* table, TransactionId id, std::istream& is = std::cin, std::ostream& os = std::cout);
    bool commit();
    bool abort();
    void releaseWLocks();

    void begin();

    string get(const string& key);

    void remove(const string& key, bool deleteIfNotExist=false);
    void update(const string& key, const string& value, bool insertIfNotExist=false);
    void insert(const string& key, const string& value, bool updateIfExist=false);  
    bool exist(const string& key);
};

