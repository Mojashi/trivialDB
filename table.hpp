#pragma once

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <memory>
#include <mutex>
#include "types.hpp"
#include "cnf.hpp"
#include "hashmap.hpp"
#include "transaction.hpp"
#include "record.hpp"

// #include <boost/lockfree/queue.hpp>

using std::map;
using std::set;
using std::string;

// template<typename T>
// using LFQueue=boost::lockfree::queue<T>;

extern const char* redoLogFile;
extern const char* dbFile;


class Transaction;
class Table {
	TimeStamp tscount = start_ts;
	
	HashMap<string, RecordPtr> data; 
	void dump(const string& fname,const string& tempName);
	void load(const string& fname);

//----thread-unsafe-----
	void applyRedoLog(const map<string, string>& writeSet,const set<string>& deleteSet);
    void upsert(const string& key, const string& val);
	bool exist(const string& key);
//----------------------

   public:
	TimeStamp getTimeStamp();
	std::mutex redoLogMtx;
	
	RecordPtr get(const string& key);
	void checkPoint();
	TransactionPtr makeTransaction(std::istream& is = std::cin, std::ostream& os = std::cout);
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

class CouldntLockResourceError : public OperationException {
	public:
	CouldntLockResourceError() : OperationException("couldn't lock resource"){}
};

class SSNCheckFailedError : public OperationException {
	public:
	SSNCheckFailedError() : OperationException("ssn check failed"){}
};