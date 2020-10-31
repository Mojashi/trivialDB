#include "table.hpp"
#include <exception>
#include <iostream>
#include <fstream>

using std::endl;
using std::cout;
int main(){
    map<string,string> emu;
    Table table;
    table.checkPoint();
    for(int i = 0; 100 > i; i++){
        Transaction t = table.makeTransaction();
        bool abort = rand() % 2;
        for(int j = 0; 100 > j; j++){
            string key = std::to_string(rand()), val = std::to_string(rand());
            int f = rand() % 5;
            try{
                if(f <= 1){
                    t.insert(key,val);
                    if(!abort) emu[key] = val;
                }
                else if(f <= 3){
                    t.update(key,val);
                    if(!abort) emu[key] = val;
                }
                else{
                    t.remove(key);
                    if(!abort) emu.erase(key);
                }
            } catch(std::exception& e){}
        }
        if(abort) t.abort();
        else t.commit();
    }
    
    std::ofstream ofs("out.txt");
	for (auto& d : emu) {
		ofs << d.first << ":" << d.second << endl;
	}
    ofs.close();

    Transaction t = table.makeTransaction();
    cout << "start" << endl;
    for(int j = 0; 1000000 > j; j++){
        try{
            if(rand() % 2){
                t.insert(std::to_string(rand()), std::to_string(rand()));
            }
            else{
                t.update(std::to_string(rand()), std::to_string(rand()));
            }
        } catch(std::exception& e){}
    }
    cout << "fin" << endl;
}