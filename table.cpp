#include "table.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cassert>
#include "utils.hpp"

using std::cerr;
using std::cout;
using std::endl;

const char* redoLogFile = "redo.log";
const char* dbFile = "data.db";
const char* dbTempFile = "data.temp";

string Table::get(const string& key) {
    if(!validateKey(key)) throw InvalidKeyError();

	if (!exist(key)) throw RecordDoesNotExistError();
	return data.at(key);
}
bool Table::exist(const string& key) {
    if(!validateKey(key)) throw InvalidKeyError();
	return data.contains(key); 
}
void Table::applyRedoLog(const map<string, string>& writeSet,
						 const set<string>& deleteSet) {
	for (auto& key : deleteSet) {
		if (data.contains(key)) data.erase(key);
	}
	for (auto& w : writeSet) {
		upsert(w.first, w.second);
	}
}
Transaction Table::makeTransaction(std::istream& is,std::ostream& os) { return Transaction(this, timestampMillseconds(), is, os); }

const std::runtime_error invalid_format_error("invalid format");
const std::runtime_error invalid_checksum_error("invalid checksum");

unsigned int readUInt(FILE* fp) {
	unsigned int v;
	if (fscanf(fp, "$%u\n", &v) != 1) {
		throw invalid_format_error;
	}
	return v;
}

string readStr(FILE* fp) {
	unsigned int len = readUInt(fp);
	if (len > MAX_TRANSACTION_SIZE) 
	    throw TooLargeTransactionError();

	string buf(len, '\0');
	char* bufp = &buf[0];
	if (fread(bufp, len, 1, fp) < 1) {
		throw invalid_format_error;
	}
	fgetc(fp);	// read \n
	return buf;
}

bool isEOF(FILE* fp) {
	bool ret = fgetc(fp) == EOF;
	fseek(fp, -1, SEEK_CUR);
	return ret;
}
void Table::checkPoint() {
	unsigned int sum, sz;

	load(dbFile);

	FILE* fp = fopen(redoLogFile, "rb");
	if (fp == NULL) {
		return;
	}
	cout << "redo.log found" << endl;
	try {
		while (!isEOF(fp)) {
			if (fscanf(fp, "$%u\n$%u\n", &sum, &sz) < 2)
				throw invalid_format_error;
			if (sz > MAX_TRANSACTION_SIZE) 
		        throw TooLargeTransactionError();
        
			map<string, string> writeSet;
			set<string> deleteSet;

			string buf(sz, '\0');
			char* bufp = &buf[0];
			if (fread(bufp, sz, 1, fp) < 1) throw invalid_format_error;

			if (sum != checksum(bufp, sz)) throw invalid_checksum_error;

			fseek(fp, -(int)sz, SEEK_CUR);
			unsigned int n = readUInt(fp);

			for (int i = 0; n > i; i++) {
				string cmd = readStr(fp);
				if (cmd == "set") {
					string key = readStr(fp);
					string val = readStr(fp);
					writeSet[key] = val;
				} else if (cmd == "delete") {
					string key = readStr(fp);
					deleteSet.insert(key);
				}
			}

			applyRedoLog(writeSet, deleteSet);
		}
	} catch (std::runtime_error& e) {
		cerr << e.what() << endl;
		cerr << "ID:? was aborted" << endl;
	}
	fclose(fp);

	dump(dbFile, dbTempFile);
	remove(redoLogFile);
}

void Table::showAll() {
	cout << endl;
	
	// for (auto& d : data) {
	// 	cout << d.first << ":" << d.second << endl;
	// }
}

void Table::dump(const string& fname, const string& tempName) {
	FILE* fp = fopen(tempName.c_str(), "wb");
	if (fp == NULL)
		throw std::runtime_error("an error occured while opening db file");

	for (auto& w : data.dump()) {
		if (fprintf(fp, "$%zu\n%s\n$%zu\n%s\n", w.first.size(), w.first.c_str(),
					w.second.size(), w.second.c_str()) < 0) {
			fclose(fp);
			throw std::runtime_error("");
		}
	}
	if (fflush(fp) == EOF) throw std::runtime_error("");
	if (fsync(fileno(fp)) == -1) throw std::runtime_error("");
	if (fclose(fp) == EOF) throw std::runtime_error("");
	if (rename(tempName.c_str(), fname.c_str()) != 0)
		throw std::runtime_error("");
}

void Table::load(const string& fname) {
	data.clear();

	FILE* fp = fopen(fname.c_str(), "rb");
	if (fp == NULL) return;

	try {
		while (!isEOF(fp)) {
			string key = readStr(fp);
			string val = readStr(fp);
			upsert(key, val);
		}
	} catch (std::runtime_error& e) {
		cerr << e.what() << endl;
	}
	fclose(fp);
}

void Table::upsert(const string& key, const string& val){
	if(!validateKey(key)) throw InvalidKeyError();
	data.set(key, val);
}