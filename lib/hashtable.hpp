//
// Created by SiriusNEO on 2021/4/29.
//

#ifndef TICKETSYSTEM_2021_MAIN_HASHTABLE_HPP
#define TICKETSYSTEM_2021_MAIN_HASHTABLE_HPP

#include "mytools.hpp"

namespace Sirius {
    
    template<int SIZE, int MOD, int SEED>
    class HashTable {
    private:
        LinkList<FixedStr<SIZE>>* table;
        int siz;
        unsigned long long hash(const FixedStr<SIZE>& nowStr, int mod, int seed) {
            unsigned long long ret = 0;
            const char* p = nowStr.str;
            while (*p) ret = (ret * seed + (*p++) - '0') % mod;
            return ret;
        }
    public:
        HashTable():siz(0){table = new LinkList<FixedStr<SIZE>> [MOD+2];}
        ~HashTable(){delete [] table;}
        void insert(const FixedStr<SIZE>& str) {
            int hash1 = hash(str, MOD, SEED);
            if (table[hash1].find(str) == -1) table[hash1].push_front(str);
            siz++;
        }
        bool find(const FixedStr<SIZE>& str) {
            int hash1 = hash(str, MOD, SEED);
            return table[hash1].find(str) != -1;
        }
        void del(const FixedStr<SIZE>& str) {
            int hash1 = hash(str, MOD, SEED);
            table[hash1].del(table[hash1].find(str));
            siz--;
        }
        void clear() {
            siz = 0;
            delete [] table;
            table = new LinkList<FixedStr<SIZE>> [MOD+2];
        }
        bool empty(){return siz==0;}
    };
}

#endif //TICKETSYSTEM_2021_MAIN_HASHTABLE_HPP
