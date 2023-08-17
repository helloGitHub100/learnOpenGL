#include "codeConvert.h"
#include <codecvt>
#include <iostream>

std::wstring UTF8ToUnicode(const std::string & str)
{
    std::wstring ret;
    try {
        std::wstring_convert< std::codecvt_utf8<wchar_t> > wcv;
        ret = wcv.from_bytes(str);
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
    return ret;
}

std::string UnicodeToANSI(const std::wstring & wstr)
{
    std::string defaultLocal = setlocale(LC_CTYPE, "");
    std::string ret;
    std::mbstate_t state = {};
    const wchar_t *src = wstr.data();
    size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if (static_cast<size_t>(-1) != len) {
        std::unique_ptr< char [] > buff(new char[len + 1]);
        len = std::wcsrtombs(buff.get(), &src, len, &state);
        if (static_cast<size_t>(-1) != len) {
            ret.assign(buff.get(), len);
        }
    }
    setlocale(LC_CTYPE, defaultLocal.c_str());
    return ret;
}

std::string UTF8ToANSI(const std::string & str)
{
    return UnicodeToANSI(UTF8ToUnicode(str));
}