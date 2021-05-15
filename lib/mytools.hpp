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
            for (int i = 0; i < SIZE; ++i)
                if (str[i] != obj.str[i]) return false;
            return true;
        }
        bool operator <= (const FixedStr<SIZE>& obj) const {return *this < obj || *this == obj;}
        bool operator > (const FixedStr<SIZE>& obj) const {return !(*this <= obj);}
        bool operator >= (const FixedStr<SIZE>& obj) const {return !(*this < obj);}
        bool operator != (const FixedStr<SIZE>& obj) const {return !(*this==obj);}

        friend std::ostream& operator << (std::ostream& os, const FixedStr& obj) {
            return os << obj.str;
        }
    };

    /* Processor */
    std::string intToString(int num) {
        std::stringstream ss;
        ss << num;
        return ss.str();
    }

    int stringToInt(const std::string& str) {
        int ret = 0;
        for (auto ch : str) ret = ret * 10 + ch - '0';
        return ret;
    }

    std::string dateFormat(int num) {
        if (num < 10) return "0"+intToString(num);
        return intToString(num);
    }

    double stringToDouble(const std::string& str) {
        double ret = 0, floatBase = 0.1;
        int i = 0;
        for (; i < str.size() && str[i] != '.'; ++i) ret = ret * 10 + str[i] - '0';
        for (++i; i < str.size(); ++i) ret += (str[i] - '0')*floatBase, floatBase *= 0.1;
        return ret;
    }

    std::string doubleToString(double num) {
        std::stringstream ss;
        ss << num;
        return ss.str();
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
}

#endif //TICKETSYSTEM_2021_MAIN_MYTOOLS_HPP
