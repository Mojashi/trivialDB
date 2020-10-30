#pragma once
#include <string>
#include <iostream>
#include <vector>
using std::vector;
using std::string;

vector<string> split(const string& str, char delim);
unsigned int checksum(const char* data, int len);