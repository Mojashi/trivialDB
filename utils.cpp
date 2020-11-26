#include "utils.hpp"
#include "cnf.hpp"
#include "types.hpp"

string getFname(TimeStamp cstamp){
    return std::to_string(cstamp) + ".log";
}

bool checkFname(TimeStamp cstamp, const string& fname){
	return split(fname, '/').back() == getFname(cstamp);
}

vector<string> split(const string& str, char delim) {
    vector<string> ret;
    size_t idx = 0, nex;
    while((nex = str.find(delim,idx + 1)) != std::string::npos){
        if(nex-idx > 0)
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

bool validateKey(const string& key){
    return (MAX_KEY_LENGTH >= key.size() && key.size() > 0 && key.find('\0') == string::npos);
}


#include <boost/chrono.hpp>

long long timestampMillseconds()
{
  using namespace boost::chrono;
  return duration_cast<milliseconds>(
      system_clock::now().time_since_epoch()).count();
}
