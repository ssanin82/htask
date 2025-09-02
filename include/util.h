#pragma once

#include <cmath>
#include <limits>
#include <string>

namespace htask {
namespace util {

const double epsilon = 0.00000001;

inline bool eq(double a, double b) {
    return fabs(a - b) < epsilon;
}

inline bool gt(double a, double b) {
    return a > b && fabs(a - b) >= epsilon;
}

inline bool lt(double a, double b) {
    return b > a && fabs(b - a) >= epsilon;
}

inline std::string normalizeNum(const std::string& s) {
    std::string s1;
    if(s.find('.') != std::string::npos) {
        s1 = s.substr(0, s.find_last_not_of('0') + 1);
        if(s1.find('.') == s1.size() - 1) s1 = s1.substr(0, s1.size()-1);
    } else return s;
    return s1;
}

}
}
