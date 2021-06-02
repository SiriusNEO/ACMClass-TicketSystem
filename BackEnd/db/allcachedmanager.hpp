//
// Created by SiriusNEO on 2021/5/24.
//

#ifndef CODE_ALLCACHEDMANAGER_HPP
#define CODE_ALLCACHEDMANAGER_HPP

#include "../lib/bst.hpp"
#include "bpt.hpp"

namespace Sirius {

    constexpr int SIZE = 900002;

    template<class keyType, class valueType>
    class ACManager {
        public:
            typedef std::pair<keyType, valueType> dataType;
            int size;
            BinarySearchTree<keyType> table;
            dataType* memoryPool[SIZE];
            Bptree<keyType, valueType> bptree;
        public:
            ACManager(const char* fileName1, const char* fileName2):bptree(fileName1, fileName2), size(0){}
            ~ACManager(){ //write back
                for (int i = 0; i < size; ++i)
                    bptree.modify(memoryPool[i]->first ,memoryPool[i]->second), delete memoryPool[i], memoryPool[i] = nullptr;
            }
            void insert(const keyType& key, const valueType& val) {
                bptree.insert(key, val);
            }
            std::pair<valueType, bool> find(const keyType& key) {
                int loc = table.find(key);
                if (loc != -1) return std::make_pair(memoryPool[loc]->second, true);
                const auto& ret = bptree.find(key);
                memoryPool[size] = new dataType (std::pair<keyType, valueType>(key, ret.first));
                table.insert(key, size);
                size++;
                return ret;
            }
            bool modify(const keyType& key, const valueType& val) {
                int loc = table.find(key);
                if (loc != -1) {
                    *memoryPool[loc] = std::make_pair(key, val);
                    return true;
                }
                return false; //error
            }
            void clear() {
                table.clear();
                bptree.clear();
                for (int i = 0; i < size; ++i) delete memoryPool[i], memoryPool[i] = nullptr;
                size = 0;
            }
    };
}


#endif //CODE_ALLCACHEDMANAGER_HPP
