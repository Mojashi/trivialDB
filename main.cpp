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
    table.checkPoint();
    
    while(1){
        cout <<  "you > ";
        string s;
        getline(std::cin, s);
        vector<string> args = split(s, ' ');

        if(args.size() == 0) continue;

        cout  << "db > ";
        if(args[0] == "begin"){
            Transaction ts = table.makeTransaction();
            ts.begin();
        } else if(args[0] == "exit"){
            table.checkPoint();
            break;
        } else if(args[0] == "help"){
            cout << "begin show exit help" << endl;
        } else if(args[0] == "show"){
            table.showAll();
        } else {
            cerr << "unknown operation" << endl;
        }
    }

}