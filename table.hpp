#pragma once
#include <map>
#include <set>
#include <string>
#include "transaction.hpp"

using std::map;
using std::set;
using std::string;

extern const char* redoLogFile;
extern const char* dbFile;

class Transaction;
class Table {
	map<string, string> data;

	void dump(const string& fname,const string& tempName);
	void load(const string& fname);

   public:
	string get(const string& key);
	bool exist(const string& key);
	void applyRedoLog(const map<string, string>& writeSet,const set<string>& deleteSet);
	void checkPoint();
	Transaction makeTransaction();
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