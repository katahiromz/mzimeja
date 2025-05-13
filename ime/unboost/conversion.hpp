// conversion.hpp --- Unboost conversion
//////////////////////////////////////////////////////////////////////////////

#ifndef UNBOOST_CONVERSION_HPP_
#define UNBOOST_CONVERSION_HPP_

#if (_MSC_VER > 1000)
    #pragma once
#endif

#include "unboost.h"
#include "exception.hpp"    // for unboost::invalid_argument, ...

#include <cstdlib>
#include <string>       // for std::string, std::wstring, std::to_string
#include <sstream>      // for std::stringstream

// If not choosed, choose one
#if ((defined(UNBOOST_USE_CXX11_CONVERSION) + defined(UNBOOST_USE_BOOST_CONVERSION) + defined(UNBOOST_USE_UNBOOST_CONVERSION)) == 0)
    #ifdef UNBOOST_USE_CXX11
        #define UNBOOST_USE_CXX11_CONVERSION
    #elif defined(UNBOOST_USE_BOOST)
        #define UNBOOST_USE_BOOST_CONVERSION
    #else
        #ifdef UNBOOST_CXX11    // C++11
            #define UNBOOST_USE_UNBOOST_CONVERSION
        #elif defined(_MSC_VER)
            #if (_MSC_VER >= 1600)
                // Visual C++ 2010 and later
                #ifndef UNBOOST_NO_CXX11
                    #define UNBOOST_USE_CXX11_CONVERSION
                #else
                    #define UNBOOST_USE_UNBOOST_CONVERSION
                #endif
            #else
                #define UNBOOST_USE_UNBOOST_CONVERSION
            #endif
        #else
            #define UNBOOST_USE_UNBOOST_CONVERSION
        #endif
    #endif
#endif

// Adapt choosed one
#ifdef UNBOOST_USE_CXX11_CONVERSION
    namespace unboost {
        template <typename T, typename U>
        inline T lexical_cast(const U& value) {
            std::stringstream stream;
            stream << value;
            if (stream.fail()) {
                throw bad_lexical_cast();
            }
            T result;
            stream >> result;
            if (stream.fail()) {
                throw bad_lexical_cast();
            }
            return result;
        }
        using std::stoi;
        using std::stol;
        using std::stoul;
        using std::stoll;
        using std::stoull;
        using std::stof;
        using std::stod;
        using std::stold;
        using std::to_string;
        using std::to_wstring;
    } // namespace unboost
#elif defined(UNBOOST_USE_BOOST_CONVERSION)
    #include <boost/lexical_cast.hpp>           // for lexical_cast
    #include <climits>      // for INT_MAX, INT_MIN, FLT_MAX, ...
    #include <cfloat>       // for FLT_MAX, ...
    #include <stdexcept>    // for std::invalid_argument, ...
    namespace unboost {
        using boost::lexical_cast;
        inline long stol(const std::string& str, size_t *pos = NULL, int base = 10) {
            long ret;
            size_t npos;
            if (pos == NULL) {
                pos = &npos;
            }
            char *end = NULL;
            ret = std::strtol(str.c_str(), &end, base);
            *pos = end - str.c_str();
            if (*pos == 0) {
                throw invalid_argument("unboost::stol");
            }
            return ret;
        }
        inline long stoul(const std::string& str, size_t *pos = NULL, int base = 10) {
            long ret;
            size_t npos;
            if (pos == NULL) {
                pos = &npos;
            }
            char *end = NULL;
            ret = std::strtoul(str.c_str(), &end, base);
            *pos = end - str.c_str();
            if (*pos == 0) {
                throw invalid_argument("unboost::stoul");
            }
            return ret;
        }
        inline int stoi(const std::string& str, size_t *pos = NULL, int base = 10) {
            long n = unboost::stol(str, pos, base);
            if (n > INT_MAX || n < INT_MIN) {
                throw out_of_range("unboost::stoi");
            }
            return static_cast<int>(n);
        }
        #ifdef UNBOOST_CXX11    // C++11
            inline _int64_t
            stoll(const std::string& str, size_t *pos = NULL, int base = 10) {
                _int64_t ret;
                size_t npos;
                if (pos == NULL) {
                    pos = &npos;
                }
                char *end = NULL;
                ret = std::strtoll(str.c_str(), &end, base);
                *pos = end - str.c_str();
                if (*pos == 0) {
                    throw invalid_argument("unboost::stoll");
                }
                return ret;
            }
            inline _uint64_t
            stoull(const std::string& str, size_t *pos = NULL, int base = 10) {
                _uint64_t ret;
                size_t npos;
                if (pos == NULL) {
                    pos = &npos;
                }
                char *end = NULL;
                ret = std::strtoull(str.c_str(), &end, base);
                *pos = end - str.c_str();
                if (*pos == 0) {
                    throw invalid_argument("unboost::stoull");
                }
                return ret;
            }
        #else   // ndef UNBOOST_CXX11
            inline _int64_t stoll(const std::string& str) {
                // TODO: support pos and base
                std::stringstream stream;
                stream << str;
                _int64_t result;
                stream >> result;
                if (stream.fail()) {
                    throw invalid_argument("unboost::stoll");
                }
                return result;
            }
            inline _uint64_t stoull(const std::string& str) {
                // TODO: support pos and base
                std::stringstream stream;
                stream << str;
                _uint64_t result;
                stream >> result;
                if (stream.fail()) {
                    throw invalid_argument("unboost::stoull");
                }
                return result;
            }
        #endif  // ndef UNBOOST_CXX11
        inline float stof(const std::string& str, size_t *pos = NULL) {
            double d;
            size_t npos;
            if (pos == NULL) {
                pos = &npos;
            }
            char *end = NULL;
            d = std::strtod(str.c_str(), &end);
            *pos = end - str.c_str();
            if (*pos == 0) {
                throw invalid_argument("unboost::stof");
            }
            if (d > FLT_MAX || d < -FLT_MAX) {
                throw out_of_range("unboost::stof");
            }
            float ret = static_cast<float>(d);
            return ret;
        }
        inline double stod(const std::string& str, size_t *pos = NULL) {
            double ret;
            size_t npos;
            if (pos == NULL) {
                pos = &npos;
            }
            char *end = NULL;
            ret = std::strtod(str.c_str(), &end);
            *pos = end - str.c_str();
            if (*pos == 0) {
                throw invalid_argument("unboost::stod");
            }
            return ret;
        }

        // NOTE: We don't use boost::to_string for C++11 compatibility.
        //using boost::to_string;
        #define UNBOOST_NEED_UNBOOST_TO_STRING
    } // namespace unboost
#elif defined(UNBOOST_USE_UNBOOST_CONVERSION)
    #include <climits>      // for INT_MAX, INT_MIN, ...
    #include <cfloat>       // for FLT_MAX, ...
    #include <cstring>      // for std::strlen, std::wcslen, ...
    #include <iostream>
    #include <sstream>    // for std::strstream
    namespace unboost {
        template <typename T, typename U>
        inline T lexical_cast(const U& value) {
            std::stringstream ss;
            ss << value;
            if (ss.fail()) {
                throw bad_lexical_cast();
            }
            T result;
            ss >> result;
            if (ss.fail()) {
                throw bad_lexical_cast();
            }
            return result;
        }
        inline long stol(const std::string& str, size_t *pos = NULL, int base = 10) {
            long ret;
            size_t npos;
            if (pos == NULL) {
                pos = &npos;
            }
            char *end = NULL;
            ret = std::strtol(str.c_str(), &end, base);
            *pos = end - str.c_str();
            if (*pos == 0) {
                throw invalid_argument("unboost::stol");
            }
            return ret;
        }
        inline long stoul(const std::string& str, size_t *pos = NULL, int base = 10) {
            long ret;
            size_t npos;
            if (pos == NULL) {
                pos = &npos;
            }
            char *end = NULL;
            ret = std::strtoul(str.c_str(), &end, base);
            *pos = end - str.c_str();
            if (*pos == 0) {
                throw invalid_argument("unboost::stoul");
            }
            return ret;
        }
        inline int stoi(const std::string& str, size_t *pos = NULL, int base = 10) {
            long n = unboost::stol(str, pos, base);
            if (n > INT_MAX || n < INT_MIN) {
                throw out_of_range("unboost::stoi");
            }
            return static_cast<int>(n);
        }
        #ifdef UNBOOST_CXX11    // C++11
            inline _int64_t
            stoll(const std::string& str, size_t *pos = NULL, int base = 10) {
                _int64_t ret;
                size_t npos;
                if (pos == NULL) {
                    pos = &npos;
                }
                char *end = NULL;
                ret = std::strtoll(str.c_str(), &end, base);
                *pos = end - str.c_str();
                if (*pos == 0) {
                    throw invalid_argument("unboost::stoll");
                }
                return ret;
            }
            inline _uint64_t
            stoull(const std::string& str, size_t *pos = NULL, int base = 10) {
                _uint64_t ret;
                size_t npos;
                if (pos == NULL) {
                    pos = &npos;
                }
                char *end = NULL;
                ret = std::strtoull(str.c_str(), &end, base);
                *pos = end - str.c_str();
                if (*pos == 0) {
                    throw invalid_argument("unboost::stoull");
                }
                return ret;
            }
        #else   // ndef UNBOOST_CXX11
            #ifndef __WATCOMC__
                inline __int64 stoll(const std::string& str) {
                    // TODO: support pos and base
                    std::stringstream ss;
                    ss << str;
                    __int64 result;
                    ss >> result;
                    if (ss.fail()) {
                        throw invalid_argument("unboost::stoll");
                    }
                    return result;
                }
                inline unsigned __int64 stoull(const std::string& str) {
                    // TODO: support pos and base
                    std::stringstream ss;
                    ss << str;
                    unsigned __int64 result;
                    ss >> result;
                    if (ss.fail()) {
                        throw invalid_argument("unboost::stoull");
                    }
                    return result;
                }
            #endif  // ndef __WATCOMC__
        #endif  // ndef UNBOOST_CXX11
        inline float stof(const std::string& str, size_t *pos = NULL) {
            using namespace std;
            double d;
            size_t npos;
            if (pos == NULL) {
                pos = &npos;
            }
            char *end = NULL;
            d = strtod(str.c_str(), &end);
            *pos = end - str.c_str();
            if (*pos == 0) {
                throw invalid_argument("unboost::stof");
            }
            if (d > FLT_MAX || d < -FLT_MAX) {
                throw out_of_range("unboost::stof");
            }
            float ret = static_cast<float>(d);
            return ret;
        }
        inline double stod(const std::string& str, size_t *pos = NULL) {
            double ret;
            size_t npos;
            if (pos == NULL) {
                pos = &npos;
            }
            char *end = NULL;
            ret = std::strtod(str.c_str(), &end);
            *pos = end - str.c_str();
            if (*pos == 0) {
                throw invalid_argument("unboost::stod");
            }
            return ret;
        }
    } // namespace unboost
    #define UNBOOST_NEED_UNBOOST_TO_STRING
#else
    #error Your compiler is not supported yet. You lose.
#endif

#ifdef UNBOOST_NEED_UNBOOST_TO_STRING
    namespace unboost {
        inline std::string to_string(int d) {
            using namespace std;
            char buf[12];
            sprintf(buf, "%d", d);
            assert(strlen(buf) < 12);
            return std::string(buf);
        }
        inline std::string to_string(unsigned int u) {
            using namespace std;
            char buf[12];
            sprintf(buf, "%u", u);
            assert(strlen(buf) < 12);
            return std::string(buf);
        }
        inline std::string to_string(long n) {
            using namespace std;
            char buf[21];
            sprintf(buf, "%ld", n);
            assert(strlen(buf) < 21);
            return std::string(buf);
        }
        inline std::string to_string(unsigned long u) {
            using namespace std;
            char buf[21];
            sprintf(buf, "%lu", u);
            assert(strlen(buf) < 21);
            return std::string(buf);
        }
        inline std::string to_string(_int64_t n) {
            using namespace std;
            char buf[21];
            #ifdef UNBOOST_OLD_COMPILER
                sprintf(buf, "%I64d", n);
            #else
                sprintf(buf, "%lld", n);
            #endif
            assert(strlen(buf) < 21);
            return std::string(buf);
        }
        inline std::string to_string(_uint64_t u) {
            using namespace std;
            char buf[21];
            #ifdef UNBOOST_OLD_COMPILER
                sprintf(buf, "%I64u", u);
            #else
                sprintf(buf, "%llu", u);
            #endif
            assert(strlen(buf) < 21);
            return std::string(buf);
        }
        inline std::string to_string(float e) {
            using namespace std;
            char buf[50];
            sprintf(buf, "%f", e);
            assert(strlen(buf) < 50);
            return std::string(buf);
        }
        inline std::string to_string(double e) {
            using namespace std;
            char buf[50];
            sprintf(buf, "%f", e);
            assert(strlen(buf) < 50);
            return std::string(buf);
        }
        inline std::string to_string(long double e) {
            using namespace std;
            char buf[310];
            sprintf(buf, "%Lf", e);
            assert(strlen(buf) < 310);
            return std::string(buf);
        }

        inline std::wstring to_wstring(int d) {
            using namespace std;
            wchar_t buf[12];
            swprintf(buf, L"%d", d);
            assert(wcslen(buf) < 12);
            return std::wstring(buf);
        }
        inline std::wstring to_wstring(unsigned int u) {
            using namespace std;
            wchar_t buf[12];
            swprintf(buf, L"%u", u);
            assert(wcslen(buf) < 12);
            return std::wstring(buf);
        }
        inline std::wstring to_wstring(long n) {
            using namespace std;
            wchar_t buf[21];
            swprintf(buf, L"%ld", n);
            assert(wcslen(buf) < 21);
            return std::wstring(buf);
        }
        inline std::wstring to_wstring(unsigned long u) {
            using namespace std;
            wchar_t buf[21];
            swprintf(buf, L"%lu", u);
            assert(wcslen(buf) < 21);
            return std::wstring(buf);
        }
        inline std::wstring to_wstring(_int64_t n) {
            using namespace std;
            wchar_t buf[21];
            #ifdef UNBOOST_OLD_COMPILER
                swprintf(buf, L"%I64d", n);
            #else
                swprintf(buf, L"%lld", n);
            #endif
            assert(wcslen(buf) < 21);
            return std::wstring(buf);
        }
        inline std::wstring to_wstring(_uint64_t u) {
            using namespace std;
            wchar_t buf[21];
            #ifdef UNBOOST_OLD_COMPILER
                swprintf(buf, L"%I64u", u);
            #else
                swprintf(buf, L"%llu", u);
            #endif
            assert(wcslen(buf) < 21);
            return std::wstring(buf);
        }
        inline std::wstring to_wstring(float e) {
            using namespace std;
            wchar_t buf[50];
            swprintf(buf, L"%f", e);
            assert(wcslen(buf) < 50);
            return std::wstring(buf);
        }
        inline std::wstring to_wstring(double e) {
            using namespace std;
            wchar_t buf[50];
            swprintf(buf, L"%f", e);
            assert(wcslen(buf) < 50);
            return std::wstring(buf);
        }
        inline std::wstring to_wstring(long double e) {
            using namespace std;
            wchar_t buf[310];
            swprintf(buf, L"%Lf", e);
            assert(wcslen(buf) < 310);
            return std::wstring(buf);
        }
    } // namespace unboost
#endif  // def UNBOOST_NEED_UNBOOST_TO_STRING

#endif  // ndef UNBOOST_CONVERSION_HPP_
