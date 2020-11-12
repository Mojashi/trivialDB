#pragma once
#include <map>
#include <set>
#include <string>
#include "cnf.hpp"
#include "transaction.hpp"
#include "hashmap.hpp"
#include <iostream>

using std::map;
using std::set;
using std::string;

extern const char* redoLogFile;
extern const char* dbFile;


class Transaction;
class Table {
	HashMap<string, string> data; 
	void dump(const string& fname,const string& tempName);
	void load(const string& fname);

   public:
    void upsert(const string& key, const string& val);
	string get(const string& key);
	bool exist(const string& key);
	void applyRedoLog(const map<string, string>& writeSet,const set<string>& deleteSet);
	void checkPoint();
	Transaction makeTransaction(std::istream& is = std::cin, std::ostream& os = std::cout);
	void showAll();
};

class OperationException : public std::exception {
	const char* s;

   public:
	OperationException(const char* s) : s(s) {}
	const char* what() const noexcept { return s; }
};
class RecordDoesNotExistError : public OperationException {
   public:
	RecordDoesNotExistError() : OperationException("doesn't exist") {}
};
class RecordExistError : public OperationException {
   public:
	RecordExistError() : OperationException("already exist") {}
};
class InvalidKeyError : public OperationException {
   public:
	InvalidKeyError() : OperationException("invalid key") {}
};
class TooLargeTransactionError : public OperationException {
   public:
	TooLargeTransactionError() : OperationException("too large transaction") {}
};