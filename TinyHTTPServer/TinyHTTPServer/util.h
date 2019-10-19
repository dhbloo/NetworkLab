#ifndef _HEADER_UTIL_
#define _HEADER_UTIL_
#pragma once

#include <string>

std::string Rfc1123DateTimeNow();

std::string UrlEncode(const std::string& str);
std::string UrlDecode(const std::string& str);

std::string& ToLower(std::string& str);
std::string& ToUpper(std::string& str);

#endif