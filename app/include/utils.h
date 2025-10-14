// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

inline std::tm stringParaTm(const std::string& s) {
    std::tm t = {};
    std::istringstream ss(s);
    ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S"); // formato do CSV
    return t;
}

inline std::string tmParaString(const std::tm& t) {
    std::ostringstream ss;
    ss << std::put_time(&t, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

#endif