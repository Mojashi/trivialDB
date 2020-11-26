#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "types.hpp"
using std::vector;
using std::string;

vector<string> split(const string& str, char delim);
unsigned int checksum(const char* data, int len);
bool validateKey(const string& key);
long long timestampMillseconds();
string getFname(TimeStamp cstamp);
bool checkFname(TimeStamp cstamp, const string& fname);