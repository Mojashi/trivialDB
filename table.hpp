#pragma once
#include <map>
#include <set>
#include <string>
#include "cnf.hpp"
#include "transaction.hpp"
#include "hashmap.hpp"
#include "record.hpp"
#include <iostream>
#include <memory>
#include <mutex>
// #include <boost/lockfree/queue.hpp>

using std::map;
using std::set;
using std::string;

using RecordPtr=std::shared_ptr<Record<string>>;
// template<typename T>
// using LFQueue=boost::lockfree::queue<T>;

extern const char* redoLogDir;
extern const char* dbFile;


class Transaction;
class Table {
	int tscount = 0;
	// LFQueue<RecordPtr> phantomLikeRecords; 
	HashMap<string, RecordPtr> data; 
//----thread-unsafe-----
	void dump(const string& fname,const string& tempName);
	void load(const string& fname);
<<<<<<< HEAD

=======
	void readRedoLog(const string& fname);
>>>>>>> 1870a45... parallel WAL
	void applyRedoLog(const map<string, string>& writeSet,const set<string>& deleteSet);
    void upsert(const string& key, const string& val);
	bool exist(const string& key);

   public:
<<<<<<< HEAD
	std::mutex redoLogMtx;
	// void addPLRecords(RecordPtr& record);
=======
	TimeStamp getTimeStamp();
	
>>>>>>> 1870a45... parallel WAL
	RecordPtr get(const string& key);
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

class CouldntLockResourceError : public OperationException {
	public:
	CouldntLockResourceError() : OperationException("couldn't lock resource"){}
};