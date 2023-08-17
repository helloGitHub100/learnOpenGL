#ifndef CODECONVERT_H
#define CODECONVERT_H
#include <string>

extern std::wstring UTF8ToUnicode(const std::string & str);

extern std::string UnicodeToANSI(const std::wstring & wstr);

extern std::string UTF8ToANSI(const std::string & str);

#endif