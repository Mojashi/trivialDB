#pragma once

#include <memory>
#include <string>

using std::string;

class Transaction;
template<typename V>
class Record;

using TransactionPtr=std::shared_ptr<Transaction>;
using RecordPtr=std::shared_ptr<Record<string>>;
using TimeStamp = unsigned long long int;
using TransactionId = TimeStamp;