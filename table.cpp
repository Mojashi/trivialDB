#include "table.hpp"
#include "cassert"

const char* redoLogFile = "redo.log";
string Table::get(const string& key) { 
    if(!exist(key)) throw RecordDoesNotExistError();
    return data[key]; 
}
bool Table::exist(const string& key) { return data.count(key) > 0; }
void Table::applyRedoLog(const map<string, string>& writeSet,
						 const set<string>& deleteSet) {
	for (auto& key : deleteSet) {
        assert(data.count(key) > 0);
        data.erase(key);
	}
    for(auto& key : writeSet){
        data[key.first] = key.second;
    }
}
Transaction Table::makeTransaction() {
    return Transaction(this, rand());
}