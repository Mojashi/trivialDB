#pragma once

#include <tbb/concurrent_hash_map.h>
#include <map>
#include <string>
#include "record.hpp"
#include <memory>

template<typename K, typename V>
class HashMap{
    using dictAccessor=typename tbb::concurrent_hash_map<K,V>::accessor;    

	tbb::concurrent_hash_map<K,V> data;

public:
    V at(K key);
    bool set(K key,const V& val);
    bool contains(K key);
    bool erase(K key);
    void clear();
    std::map<K,V> dump();
};

template class HashMap<std::string,std::shared_ptr<Record<std::string>>>;
