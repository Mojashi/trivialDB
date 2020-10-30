#include "transaction.hpp"
#include "utils.hpp"
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <stdexcept>
using std::cerr;
using std::endl;
using std::cout;

Transaction::Transaction(Table* table, int id) : table(table),id(id) {}

void Transaction::begin() {
    cout << "TransactionID:" << id << endl;
	while (!commited && !aborted) {
        cout <<  "you > ";
		string s;
		getline(std::cin, s);
		vector<string> args = split(s, ' ');
        if(args.size() == 0) continue;

        cout << id << " > ";
		try {
			if (args[0] == "update") {
				if (args.size() < 3) throw std::invalid_argument("Not enough arguments");
				update(args[1], args[2]);
                cout << "OK" << endl;
			}
			else if (args[0] == "insert") {
				if (args.size() < 3) throw std::invalid_argument("Not enough arguments");
				insert(args[1], args[2]);
                cout << "OK" << endl;
			}
			else if (args[0] == "delete") {
				if (args.size() < 2) throw std::invalid_argument("Not enough arguments");
				remove(args[1]);
                cout << "OK" << endl;
			}
			else if (args[0] == "read") {
				if (args.size() < 2) throw std::invalid_argument("Not enough arguments");
				cout << get(args[1]) << endl;
			}
			else if (args[0] == "commit") {
                commit();
                cout << "successfully commited" << endl;
			}
			else if (args[0] == "abort") {
                abort();
                cout << "successfully aborted" << endl;
            }
            else {
                cout << "unknown operation" << endl;
            }
		} catch (OperationException& e) {
			cerr << e.what() << endl;
		} catch (std::invalid_argument& e){
            cerr << e.what() << endl;
        }
	}
}

string Transaction::get(const string& key ) {
    string ret;
    if(readSet.count(key) > 0)
        ret = readSet[key];
    else if(writeSet.count(key) > 0)
        ret = writeSet[key];
    else ret = table->get(key);
    return readSet[key] = ret;
}

void Transaction::remove(const string& key , bool deleteIfNotExist) {
    if(!deleteIfNotExist && !exist(key)) 
        throw RecordDoesNotExistError();
    deleteSet.insert(key);
    writeSet.erase(key);
    readSet.erase(key);
}
void Transaction::update(const string& key , const string& value , bool insertIfNotExist) {
    remove(key, insertIfNotExist);
    insert(key, value);
}

void Transaction::insert(const string& key , const string& value , bool updateIfExist) {
    if(!updateIfExist && exist(key)) 
        throw RecordExistError();
    deleteSet.erase(key);
    writeSet[key] = value;
    readSet[key] = value;
}

bool Transaction::exist(const string& key ){
    return !(deleteSet.count(key) > 0) && (writeSet.count(key) > 0 || table->exist(key));
}

void Transaction::writeRedoLog(const string& fname){
    string redolog = "$" + std::to_string(deleteSet.size() + writeSet.size()) + "\r\n";
    
    for(auto w : deleteSet){
        redolog += "$6\r\ndelete\r\n$" + std::to_string(w.size()) + "\r\n" + w + "\r\n";
    }
    for(auto w : writeSet){
        redolog += "$6\r\nupdate\r\n$" + std::to_string(w.first.size()) + "\r\n" + w.first + "\r\n$" + std::to_string(w.second.size())+ "\r\n" + w.second + "\r\n";
    }

    unsigned int sum = checksum(redolog.c_str(), redolog.size());
    redolog = "$" + std::to_string(sum) + "\r\n$" + std::to_string(redolog.size()) + "\r\n" + redolog;
    
    int fd = open(fname.c_str(), O_WRONLY | O_APPEND);
    if(fd == -1){
        throw std::runtime_error("an error occurred while opening file");
    }
    //write中とかにエラー起きると永続化されてるのかされてないのかわからないので一回落として再起動時のcrash recoveryに託す
    if(write(fd, redolog.c_str(), redolog.size()) == -1){
        cerr << "an error occurred while writing to file" << endl;
        exit(1);
    }
    if(fsync(fd) == -1){
        cerr << "an error occurred while syncing file" << endl;
        exit(1);
    }
    if(close(fd) == -1){
        cerr << "an error occurred while closing file" << endl;
        exit(1);
    }
}

bool Transaction::commit(){
    writeRedoLog(redoLogFile);
    table->applyRedoLog(writeSet, deleteSet);
    commited = true;
}

bool Transaction::abort(){
    aborted = true;
}