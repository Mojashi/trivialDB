#include "hashmap.hpp"
#include <stdexcept>
#include <map>

template<typename K, typename V>
V HashMap<K,V>::at(K key){
    dictAccessor accessor;
    
    bool f = data.find(accessor, key);
    if(!f) throw std::out_of_range(""); 
    return accessor->second;
}

template<typename K, typename V>
bool HashMap<K,V>::contains(K key){
    dictAccessor accessor;
    return data.find(accessor, key);
}

template<typename K, typename V>
bool HashMap<K,V>::set(K key, const V& val){
    dictAccessor accessor;
    bool ret = data.insert(accessor, key);
    accessor->second = val;
    return ret;
}

template<typename K, typename V>
bool HashMap<K,V>::erase(K key){
    return data.erase(key);
}

template<typename K, typename V>
std::map<K,V> HashMap<K,V>::dump(){
    std::map<K,V> ret;
    for(auto itr : data){
        ret[itr.first] = itr.second;
    }
    return ret;
}

template<typename K, typename V>
void HashMap<K,V>::clear(){
    data.clear();
}