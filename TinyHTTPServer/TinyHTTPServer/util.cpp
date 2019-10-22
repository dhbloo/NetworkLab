#include "util.h"
#include <algorithm>
#include <ctime>
#include <cassert>

static const char* DAY_NAMES[] =
{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char* MONTH_NAMES[] =
{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

std::string Rfc1123DateTimeNow() {
    const int RFC1123_TIME_LEN = 29;
    char buf[RFC1123_TIME_LEN + 1];
    time_t t;
    ::tm tm;

    time(&t);
    gmtime_s(&tm, &t);

    strftime(buf, RFC1123_TIME_LEN + 1, "---, %d --- %Y %H:%M:%S GMT", &tm);
    memcpy(buf, DAY_NAMES[tm.tm_wday], 3);
    memcpy(buf + 8, MONTH_NAMES[tm.tm_mon], 3);

    return buf;
}

std::string TimeNow() {
    const int TIME_LEN = 9;
    char buf[TIME_LEN];
    time_t t;
    ::tm tm;

    time(&t);
    localtime_s(&tm, &t);
    strftime(buf, TIME_LEN, "%H:%M:%S", &tm);

    return buf;
}


unsigned char ToHex(unsigned char x) {
    return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x) {
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else { assert(0); y = 0; }
    return y;
}

std::string UrlEncode(const std::string& str) {
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++) {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}

std::string UrlDecode(const std::string& str) {
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++) {
        if (str[i] == '+') strTemp += ' ';
        else if (str[i] == '%') {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high * 16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}

std::string& ToLower(std::string& str) {
    std::transform(
        str.begin(), 
        str.end(), 
        str.begin(),
        [](unsigned char c) -> unsigned char { return std::tolower(c); });
    return str;
}

std::string& ToUpper(std::string& str) {
    std::transform(
        str.begin(),
        str.end(),
        str.begin(),
        [](unsigned char c) -> unsigned char { return std::toupper(c); });
    return str;
}
