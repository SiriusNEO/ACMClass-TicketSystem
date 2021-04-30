//
// Created by SiriusNEO on 2021/4/26.
//

#ifndef TICKETSYSTEM_2021_MAIN_TOOLFUNCTIONS_HPP
#define TICKETSYSTEM_2021_MAIN_TOOLFUNCTIONS_HPP

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
    /* Type Cast */
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

#endif //TICKETSYSTEM_2021_MAIN_TOOLFUNCTIONS_HPP
