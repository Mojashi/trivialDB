#pragma once
#include <map>
#include <string>
#include <set>
#include "table.hpp"
using std::map;
using std::string;
using std::set;
class Table;
class Transaction{
    int id;
    bool commited = false, aborted = false;
    map<string,string> writeSet, readSet;
    set<string> deleteSet;
    Table* table;

public:
    Transaction(Table* table, int id);
    bool commit();
    bool abort();

    void begin();

    string get(const string& key);

    void remove(const string& key, bool deleteIfNotExist=false);
    void update(const string& key, const string& value, bool insertIfNotExist=false);
    void insert(const string& key, const string& value, bool updateIfExist=false);  
    bool exist(const string& key);
    void writeRedoLog(const string& fname);
};

