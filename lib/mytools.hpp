//
// Created by SiriusNEO on 2021/5/14.
//

#ifndef TICKETSYSTEM_2021_MAIN_MYTOOLS_HPP
#define TICKETSYSTEM_2021_MAIN_MYTOOLS_HPP

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>

//for debug
#define BOMB std::cout<<"bomb\n";
#define MINE(_x) std::cout<<_x<<'\n';
#define STANDINGBY() clock_t st = clock();
#define COMPLETE(_x) printf(_x": %.6lf\n", (clock()-st)/(double)CLOCKS_PER_SEC);

namespace Sirius {
    constexpr int Int_Max = 0x7fffffff;
    constexpr unsigned long long LL_Max = (1ll << 60) + 7;

    typedef unsigned long long hashCode;

    /* FixedStr */
    template<int SIZE>
    struct FixedStr { /* Warning: Don't use too long string to initialize it. */
        char str[SIZE];
        FixedStr():str() {}
        FixedStr(const std::string& _str):str() {
            strcpy(str, _str.c_str());
        }
        FixedStr(const char* _str):str() {
            strcpy(str, _str);
        }
        char& operator [] (int pos) {
            return str[pos];
        }
        bool operator < (const FixedStr<SIZE>& obj) const {
            return strcmp(str, obj.str) < 0;
        }
        bool operator == (const FixedStr<SIZE>& obj) const {
            return strcmp(str, obj.str) == 0;
        }
        bool operator <= (const FixedStr<SIZE>& obj) const {return *this < obj || *this == obj;}
        bool operator > (const FixedStr<SIZE>& obj) const {return !(*this <= obj);}
        bool operator >= (const FixedStr<SIZE>& obj) const {return !(*this < obj);}
        bool operator != (const FixedStr<SIZE>& obj) const {return !(*this==obj);}

        friend std::ostream& operator << (std::ostream& os, const FixedStr& obj) {
            return os << obj.str;
        }
    };

    /* LinkList */
    template<class T>
    class LinkList {
    public:
        struct Node {
            T data;
            Node *nxt;
            Node():data(), nxt(nullptr) {}
            explicit Node(const T& _data):data(_data), nxt(nullptr){}
        };
        int siz;
        Node* head;
        LinkList():head(), siz(0){head = new Node();}
        ~LinkList(){while(siz)del(1);delete head;}
        void push_front(const T& _data) {
            Node *newNode = new Node(_data);
            newNode->nxt = head->nxt, head->nxt = newNode;
            siz++;
        }
        int insert(int pos, const T& _data) {
            Node *newNode = new Node(_data), *nowNode = head;
            int ret = pos+1;
            while (nowNode->nxt && pos--) nowNode = nowNode->nxt;
            newNode->nxt = nowNode->nxt, nowNode->nxt = newNode;
            siz++;
            return ret;
        }
        void del(int pos) {
            Node *nowNode = head;
            while (nowNode->nxt && --pos) nowNode = nowNode->nxt;
            Node *targetNode = nowNode->nxt;
            nowNode->nxt = nowNode->nxt->nxt;
            delete targetNode;
            siz--;
        }
        int find(const T& _data) {
            Node* nowNode = head->nxt;
            int ret = 0;
            while (nowNode) {
                ++ret;
                if (nowNode->data == _data) return ret;
                nowNode = nowNode->nxt;
            }
            return -1;
        }
        int size(){return siz;}
    };

    /* Processor */
    int stringToInt(const std::string& str) {
        int ret = 0;
        for (auto ch : str) ret = ret * 10 + ch - '0';
        return ret;
    }

    std::string dateFormat(int num) {
        if (num < 10) return "0"+std::to_string(num);
        return std::to_string(num);
    }

    void split(const std::string& originStr, std::string ret[], int& retc, char delim) {
        retc = 0;
        for (int i = 0, j = 0; i < originStr.size(); ) {
            while (j < originStr.size() && originStr[j] != delim) ++j;
            ret[retc].clear();
            ret[retc++] = originStr.substr(i, j-i);
            while (j < originStr.size() && originStr[j] == delim) ++j;
            i = j;
        }
    }

    /* Char Validator */
    inline bool isDigit(char ch) {return ch>='0'&&ch<='9';}
    inline bool isUpperLetter(char ch) {return ch>='A'&&ch<='Z';}
    inline bool isLowerLetter(char ch) {return ch>='a'&&ch<='z';}
    inline bool isLetter(char ch) {return isUpperLetter(ch)||isLowerLetter(ch);}

    /* Algorithm */
    template <class T>
    void qsort(T* l, T* r, bool (*cmp)(const T&, const T&)) {
        if (l >= r) return;
        T *i = l, *j = r, x = *i;
        while (i < j) {
            while (i < j && cmp(x, *j)) j--;
            if (i < j) *i = *j, i++;
            while (i < j && cmp(*i, x)) i++;
            if (i < j) *j = *i, j--;
        }
        *i = x;
        qsort(l, i-1, cmp); qsort(i+1, r, cmp);
    }

    /* Random */
    inline int randInt(int l, int r) {
        return rand()%(r-l+1)+l;
    }
    inline std::string randString(int size = randInt(1, 1000)) {
        std::string  ret;
        while (size--) {
            int typ = randInt(1, 64);
            if (typ >= 1 && typ <= 10) ret += (char)randInt('0', '9');
            else if (typ >= 11 && typ <= 36) ret += (char)randInt('a', 'z');
            else if (typ >= 37 && typ <= 62) ret += (char)randInt('A', 'Z');
            else if (typ == 63) ret += '@';
            else if (typ == 64) ret += '_';
        }
        return ret;
    }

    /* Hash */
    constexpr int HashSeed = 131;
    hashCode hash(const char* nowStr) {
        unsigned long long ret = 0;
        const char* p = nowStr;
        while (*p) ret = ret * HashSeed + (*p++) - '0';
        return ret % LL_Max;
    }

    /* Fastout */
    void write(const char* str) {
        fwrite(str, sizeof(char), strlen(str), stdout);
    }

    void writeInt(int x) {
        if (!x) {putchar('0');return;}
        char ret[33];
        int p = 0;
        while (x) ret[p++] = (x%10)+48, x /= 10;
        while (p) --p, putchar(ret[p]);
    }
}

#endif //TICKETSYSTEM_2021_MAIN_MYTOOLS_HPP
