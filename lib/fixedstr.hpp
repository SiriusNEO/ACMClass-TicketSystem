//
// Created by SiriusNEO on 2021/4/29.
//

#ifndef TICKETSYSTEM_2021_MAIN_FixedStr_HPP
#define TICKETSYSTEM_2021_MAIN_FixedStr_HPP

namespace Sirius {
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
        bool operator < (const FixedStr<SIZE>& obj) const {
            for (int i = 0; i < SIZE; ++i) if (str[i] != obj.str[i]) return str[i] < obj.str[i];
            return false;
        }
        bool operator == (const FixedStr<SIZE>& obj) const {
            for (int i = 0; i < SIZE; ++i)
                if (str[i] != obj.str[i]) return false;
            return true;
        }
        bool operator != (const FixedStr<SIZE>& obj) const {return !(*this==obj);}

        friend std::ostream& operator << (std::ostream& os, const FixedStr& obj) {
            return os << obj.str;
        }
    };
}

#endif //TICKETSYSTEM_2021_MAIN_FixedStr_HPP
