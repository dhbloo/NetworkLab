#ifndef _HEADER_UTIL_
#define _HEADER_UTIL_
#pragma once

#include <string>
#include <ctime>
#include <chrono>

std::string Rfc1123DateTime(time_t t);
std::string Rfc1123DateTimeNow();
std::string TimeNow();

std::string UrlEncode(const std::string &str);
std::string UrlDecode(const std::string &str);

std::string &ToLower(std::string &str);
std::string &ToUpper(std::string &str);

template<typename TP> std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp =
        time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
}

#endif