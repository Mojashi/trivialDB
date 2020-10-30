#include <iostream>
#include <vector>
#include "table.hpp"
#include "utils.hpp"
using std::vector;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;

int main(){
    Table table;
    
    while(1){
        string s;
        getline(std::cin, s);
        vector<string> args = split(s, ' ');

        if(args.size() == 0) continue;

        if(args[0] == "begin"){
            Transaction ts = table.makeTransaction();
            ts.begin();
        } else {
            cerr << "unknown operation" << endl;
        }
    }

}