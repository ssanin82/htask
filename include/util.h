#pragma once

#include <cmath>
#include <limits>
#include <string>

namespace htask {
namespace util {

using std::string;

constexpr double epsilon = 0.00000001;
constexpr int PRICE_SCALE = 2;
constexpr int SIZE_SCALE = 8;
constexpr int NO_SIZE = 0ULL;

using PRICE_T = uint32_t;
using SIZE_T = uint64_t;

inline bool eq(double a, double b) {
    return fabs(a - b) < epsilon;
}

inline bool gt(double a, double b) {
    return a > b && fabs(a - b) >= epsilon;
}

inline bool lt(double a, double b) {
    return b > a && fabs(b - a) >= epsilon;
}

inline string normalizeNum(const string& s) {
    string s1;
    if(s.find('.') != string::npos) {
        s1 = s.substr(0, s.find_last_not_of('0') + 1);
        if(s1.find('.') == s1.size() - 1) s1 = s1.substr(0, s1.size()-1);
    } else return s;
    return s1;
}

inline long long scale_up(long long x, int y) {
    long long mul = 1;
    for (int i = 0; i < y; ++i) mul *= 10LL;
    return x * mul;
}

inline double scale_down(long long x, int y) {
    double res = x;
    for (int i = 0; i < y; ++i) res /= 10.;
    return res;
}

inline int str_to_scaled_num(const string& s, int scale) {
    int x = 0;
    int y = 0;
    size_t dpos = s.find('.');
    if(dpos != string::npos) {
        x = std::stoi(s.substr(0, dpos));
        string srem = s.substr(dpos + 1);
        if (srem.length() < scale) srem += string(scale - srem.length(), '0');
        y = std::stoi(srem);
    } else x = std::stoi(s);
    int res = scale_up(x, scale);
    res += y;
    return res;
}

}
}
