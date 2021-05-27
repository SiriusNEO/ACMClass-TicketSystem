#ifndef SJTU_UTILITY_HPP
#define SJTU_UTILITY_HPP

#include <utility>

namespace sjtu {

    template<class U1, class U2>
    class pair {
    public:
        U1 first;
        U2 second;
        constexpr pair() : first(), second() {}
        pair(const pair &other) = default;
        pair(pair &&other) = default;
        pair(const U1 &x, const U2 &y) : first(x), second(y) {}
        //template<class U1, class U2>
        pair(U1 &x, U2 &y) : first(x), second(y) {}
        pair(U1 &&x, U2 &&y) : first(x), second(y) {}
        pair(const U1 &&x, const U2 &&y) : first(x), second(y) {}
        //template<class U1, class U2>
        //pair(const pair<U1, U2> &other) : first(other.first), second(other.second) {}
        //template<class U1, class U2>
        //pair(pair<U1, U2> &&other) : first(other.first), second(other.second) {}
        //template<class U1, class U2>
        pair<U1, U2>& operator=(const pair<U1, U2> &other) {
            first = other.first; second = other.second;
            return *this;
        }

       // template<class U1, class U2>
        pair<U1, U2>& operator=(pair<U1, U2> &other) {
            first = other.first; second = other.second;
            return *this;
        }
    };

}

#endif
