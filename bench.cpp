#include <iostream>
#include <memory>
#include <thread>
#include "transaction.hpp"
#include "table.hpp"
#include <assert.h>
#include <vector>
#include <time.h>
#include <boost/iostreams/stream.hpp>
#include <chrono>

using namespace std;

const bool BENCH = true;
int KEYRANGE;

int num_thread,num_ts, bend;
float rrate;

Table table;
std::shared_ptr<Transaction> ts = table.makeTransaction();

volatile bool start = false;

volatile int commitCou = 0, abortCou = 0;

struct Xor128{
    unsigned long x=123456789,y=362436069,z=521288629,w=88675123;

    Xor128(unsigned long seed){
        w += seed;
    }

    unsigned long get(){
        unsigned long t;
        t=(x^(x<<11));x=y;y=z;z=w; return( w=(w^(w>>19))^(t^(t>>8)) );
    }

    unsigned long getBendRand(int r, int b){
        unsigned long ret = ULONG_MAX;
        for(int i = 0; b > i; i++){
            ret = min(ret, get() % r);
        }
        return ret;
    }

    string getRandomStr(int b){
        return to_string(getBendRand(KEYRANGE, b));
    }

    float getP(){
        return (get() % 10001)/10000.0;
    }
};



boost::iostreams::stream< boost::iostreams::null_sink > nullStream( ( boost::iostreams::null_sink() ) );

void worker(int id){
    while(start == false);
    Xor128 rng(id);
    for(int i = 0; num_ts > i; i++){
        TransactionPtr ts = table.makeTransaction(cin,nullStream);
        int length = 1 << rng.getBendRand(6, 1);
        for(int j = 0; length > j && ts->status() != Transaction::ABORTED; j++){
            try{
                string key = rng.getRandomStr(bend);

                if(rng.getP() < rrate){
                    ts->get(key);
                }else {
                    string value = rng.getRandomStr(bend);
                    ts->update(key, value);
                }
            } catch (CouldntLockResourceError& e) {
                ts->abort();
    		} catch (SSNCheckFailedError& e) {
                ts->abort();
	    	} catch(exception& e){}
        }
        try{
            if(ts->status() == Transaction::INFLIGHT)
                ts->commit();
    	} catch (SSNCheckFailedError& e) {
            ts->abort();
    	}
        ts->freeMem();

        if(ts->status() == Transaction::ABORTED) __sync_fetch_and_add(&abortCou, 1);
        else if(ts->status() == Transaction::COMMITTED) __sync_fetch_and_add(&commitCou, 1);
        else assert(true);
    }
}

void initTable(){
    TransactionPtr ts = table.makeTransaction();
    for(int i = 0; KEYRANGE > i; i++){
        ts->insert(to_string(i), to_string(i));
    }
    ts->commit();
}

int main(int argc, char *argv[]){
    if(argc < 6) cin >> num_thread >> num_ts >> rrate >> bend >> KEYRANGE;
    else {
        num_thread = atoi(argv[1]);
        num_ts = atoi(argv[2]);
        rrate = atof(argv[3]);
        bend = atoi(argv[4]);
        KEYRANGE = atoi(argv[5]);
    }

    assert(bend > 0);
    initTable();

    vector<thread> threads;

    for(int i = 0; num_thread > i; i++){
        threads.push_back(thread(worker, i));
    }
    
    auto st = chrono::system_clock::now();
    cout << "start" << endl;
    start = true;
    for(auto& thread : threads){
        thread.join();
    }
    cout << "en" << endl;
    auto en = chrono::system_clock::now();


    float duration = chrono::duration_cast<chrono::milliseconds>(en-st).count() / 1000.0;
    float tps = (commitCou) / duration;
    float abortRate = 1.0 * abortCou / (abortCou + commitCou);
    printf("duration:%fsec tps:%f aborted:%d committed:%d abortRate:%f\n", duration, tps, abortCou, commitCou, abortRate);
    return 0;
}