#include "utils.hpp"
vector<string> split(const string& str, char delim) {
    vector<string> ret;
    size_t idx = 0, nex;
    while((nex = str.find(delim,idx + 1)) != std::string::npos){
        ret.push_back(str.substr(idx, nex - idx));
        idx = nex + 1;
    }
    if(idx < str.size())
        ret.push_back(str.substr(idx, str.size() - idx));
    return ret;
}

unsigned int checksum(const char* data, int len){
    unsigned int sum = 0, mod = ((unsigned int)1 << 31) - 1;
    
    for(int i = 0; len > i; i++){
        sum += data[i];
        sum &= mod;
    }
    return sum;
}