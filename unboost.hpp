// unboost.hpp --- Unboost by katahiromz <katayama.hirofumi.mz@gmail.com>
// This file is public domain software (PDS).
//////////////////////////////////////////////////////////////////////////////

#ifndef UNBOOST_HPP_
#define UNBOOST_HPP_    10 // Version 10

#ifndef __cplusplus
    #error Unboost needs C++ compiler. You lose.
#endif

#include <cassert>

//////////////////////////////////////////////////////////////////////////////
// Unboost configuration (optional)

#ifdef UNBOOST_HAVE_CONFIG_HPP
    #include "unboost_config.hpp"
#endif

//////////////////////////////////////////////////////////////////////////////
// Use all?

#ifdef UNBOOST_USE_ALL
    #ifndef UNBOOST_USE_STATIC_ASSERT
        #define UNBOOST_USE_STATIC_ASSERT
    #endif
    #ifndef UNBOOST_USE_SMART_PTR
        #define UNBOOST_USE_SMART_PTR
    #endif
    #ifndef UNBOOST_USE_THREAD
        #define UNBOOST_USE_THREAD
    #endif
    #ifndef UNBOOST_USE_ARRAY
        #define UNBOOST_USE_ARRAY
    #endif
    #ifndef UNBOOST_USE_REGEX
        #define UNBOOST_USE_REGEX
    #endif
    #ifndef UNBOOST_USE_CONVERSION
        #define UNBOOST_USE_CONVERSION
    #endif
    #ifndef UNBOOST_USE_COMPLEX_FUNCTIONS
        #define UNBOOST_USE_COMPLEX_FUNCTIONS
    #endif
    #ifndef UNBOOST_USE_RANDOM
        #define UNBOOST_USE_RANDOM
    #endif
    #ifndef UNBOOST_USE_CHRONO
        #define UNBOOST_USE_CHRONO
    #endif
    #ifndef UNBOOST_USE_UNORDERED_SET
        #define UNBOOST_USE_UNORDERED_SET
    #endif
    #ifndef UNBOOST_USE_UNORDERED_MAP
        #define UNBOOST_USE_UNORDERED_MAP
    #endif
    #ifndef UNBOOST_USE_FOREACH
        #define UNBOOST_USE_FOREACH
    #endif
    #ifndef UNBOOST_USE_TUPLE
        #define UNBOOST_USE_TUPLE
    #endif
    #ifndef UNBOOST_USE_STRING_ALGORITHM
        #define UNBOOST_USE_STRING_ALGORITHM
    #endif
    #ifndef UNBOOST_USE_ASSERT
        #define UNBOOST_USE_ASSERT
    #endif
#endif

//////////////////////////////////////////////////////////////////////////////
// Is Unboost on C++11? (UNBOOST_CXX11)

#ifndef UNBOOST_CXX11
    #if (__cplusplus >= 201103L)    // C++11
        #define UNBOOST_CXX11
    #endif
#endif

//////////////////////////////////////////////////////////////////////////////
// consistency checks

#if ((defined(UNBOOST_USE_CXX11) + defined(UNBOOST_USE_TR1) + defined(UNBOOST_USE_BOOST)) >= 2)
    #error Choose one or none of UNBOOST_USE_CXX11, UNBOOST_USE_TR1 and UNBOOST_USE_BOOST. You lose.
#endif

//////////////////////////////////////////////////////////////////////////////
// static_assert

#ifdef UNBOOST_USE_STATIC_ASSERT
    // If not choosed, choose one
    #if ((defined(UNBOOST_USE_CXX11_STATIC_ASSERT) + defined(UNBOOST_USE_UNBOOST_STATIC_ASSERT) + defined(UNBOOST_USE_BOOST_STATIC_ASSERT)) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_STATIC_ASSERT
        #elif defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_STATIC_ASSERT
        #else
            #ifdef UNBOOST_CXX11   // C++11
                #define UNBOOST_USE_CXX11_STATIC_ASSERT
            #else
                #define UNBOOST_USE_UNBOOST_STATIC_ASSERT
            #endif
        #endif
    #endif
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_STATIC_ASSERT
        // static_assert is available
        #define UNBOOST_STATIC_ASSERT_MSG static_assert
    #elif defined(UNBOOST_USE_BOOST_STATIC_ASSERT)
        // Use Boost BOOST_STATIC_ASSERT_MSG
        #include <boost/static_assert.hpp>
        #define UNBOOST_STATIC_ASSERT_MSG BOOST_STATIC_ASSERT_MSG
        #ifndef static_assert
            #define static_assert BOOST_STATIC_ASSERT_MSG
        #endif
    #else
        #define UNBOOST_STATIC_ASSERT_MSG(x,y) \
            typedef UNBOOST_STATIC_ASSERTION[(x) ? 1 : -1]
        #define static_assert(x,y) UNBOOST_STATIC_ASSERT_MSG((x),(y))
    #endif
#endif

//////////////////////////////////////////////////////////////////////////////
// smart pointers

#ifdef UNBOOST_USE_SMART_PTR
    // If not choosed, choose one
    #if ((defined(UNBOOST_USE_CXX11_SMART_PTR) + defined(UNBOOST_USE_TR1_SMART_PTR) + defined(UNBOOST_USE_BOOST_SMART_PTR)) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_SMART_PTR
        #elif defined(UNBOOST_USE_TR1)
            #define UNBOOST_USE_TR1_SMART_PTR
        #elif defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_SMART_PTR
        #else
            #ifdef UNBOOST_CXX11    // C++11
                #define UNBOOST_USE_CXX11_SMART_PTR
            #elif defined(_MSC_VER)
                #if (1500 <= _MSC_VER) && (_MSC_VER <= 1600)
                    // Visual C++ 2008 SP1 and 2010
                    #define UNBOOST_USE_TR1_SMART_PTR
                    // NOTE: On MSVC 2008, you needs SP1.
                #elif (_MSC_VER >= 1700)
                    // Visual C++ 2012 and later
                    #define UNBOOST_USE_CXX11_SMART_PTR
                #else
                    // Boost
                    #define UNBOOST_USE_BOOST_SMART_PTR
                #endif
            #elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
                // GCC 4.3 and later
                #define UNBOOST_USE_TR1_SMART_PTR
            #else
                // Boost
                #define UNBOOST_USE_BOOST_SMART_PTR
            #endif
        #endif  // ndef UNBOOST_USE_CXX11
    #endif
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_SMART_PTR
        #include <memory>       // for std::shared_ptr, ...
        namespace unboost {
            using std::shared_ptr;
            using std::make_shared;
            using std::static_pointer_cast;
            using std::dynamic_pointer_cast;
            using std::unique_ptr;
            using std::weak_ptr;
        } // namespace unboost
    #elif defined(UNBOOST_USE_TR1_SMART_PTR)
        #ifdef _MSC_VER
            #include <memory>       // for std::tr1::shared_ptr, ...
        #else
            #include <tr1/memory>   // for std::tr1::shared_ptr, ...
        #endif
        namespace unboost {
            using std::tr1::shared_ptr;
            template <typename T>
            inline shared_ptr<T> make_shared(const T& value) {
                shared_ptr<T> ptr(new T(value));
                return ptr;
            }
            using std::tr1::static_pointer_cast;
            using std::tr1::dynamic_pointer_cast;
            // NOTE: There is no unique_ptr for TR1
            using std::tr1::weak_ptr;
        } // namespace unboost
    #elif defined(UNBOOST_USE_BOOST_SMART_PTR)
        #include <boost/shared_ptr.hpp>
        #include <boost/make_shared.hpp>
        #include <boost/interprocess/smart_ptr/unique_ptr.hpp>
        #include <boost/checked_delete.hpp>
        #include <boost/weak_ptr.hpp>
        #ifdef UNBOOST_FIX_UNIQUE_PTR
            namespace boost {
                namespace interprocess {
                    template<typename T>
                    struct default_delete : checked_deleter<T> {};
                    template<typename T>
                    struct default_delete<T[]> : checked_array_deleter<T> {};
                    template<typename T, typename D = default_delete<T> >
                    class unique_ptr;
                } // namespace interprocess
            } // namespace boost
        #endif  // def UNBOOST_FIX_UNIQUE_PTR
        namespace unboost {
            using boost::shared_ptr;
            using boost::make_shared;
            using boost::static_pointer_cast;
            using boost::dynamic_pointer_cast;
            using boost::interprocess::unique_ptr;
            using boost::weak_ptr;
        } // namespace unboost
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif  // def UNBOOST_USE_SMART_PTR

//////////////////////////////////////////////////////////////////////////////
// modern thread

#ifdef UNBOOST_USE_THREAD
    // If not choosed, choose one
    #if (defined(UNBOOST_USE_CXX11_THREAD) + defined(UNBOOST_USE_BOOST_THREAD) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_THREAD
        #elif defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_THREAD
        #else
            #ifdef UNBOOST_CXX11    // C++11
                #define UNBOOST_USE_CXX11_THREAD
            #elif defined(_MSC_VER)
                #if (_MSC_VER >= 1800)
                    // Visual C++ 2013 and later
                    #define UNBOOST_USE_CXX11_THREAD
                #else
                    #define UNBOOST_USE_BOOST_THREAD
                #endif
            #else
                #define UNBOOST_USE_BOOST_THREAD
            #endif
        #endif
    #endif
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_THREAD
        #include <thread>
        namespace unboost {
            using std::thread;
        }
    #elif defined(UNBOOST_USE_BOOST_THREAD)
        #include <boost/thread.hpp>
        namespace unboost {
            using boost::thread;
        }
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif

//////////////////////////////////////////////////////////////////////////////
// modern array

#ifdef UNBOOST_USE_ARRAY
    // If not choosed, choose one
    #if (defined(UNBOOST_USE_CXX11_ARRAY) + defined(UNBOOST_USE_TR1_ARRAY) + defined(UNBOOST_USE_BOOST_ARRAY) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_ARRAY
        #elif defined(UNBOOST_USE_TR1)
            #define UNBOOST_USE_TR1_ARRAY
        #elif defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_ARRAY
        #else
            #ifdef UNBOOST_CXX11    // C++11
                #define UNBOOST_USE_CXX11_ARRAY
            #elif defined(_MSC_VER)
                #if (_MSC_VER >= 1600)
                    // Visual C++ 2010 and later
                    #define UNBOOST_USE_CXX11_ARRAY
                #elif (1500 <= _MSC_VER) && (_MSC_VER < 1600)
                    // Visual C++ 2008
                    #define UNBOOST_USE_TR1_ARRAY
                #else
                    #define UNBOOST_USE_BOOST_ARRAY
                #endif
            #elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
                // GCC 4.3 and later
                #define UNBOOST_USE_TR1_ARRAY
            #else
                #define UNBOOST_USE_BOOST_ARRAY
            #endif
        #endif
    #endif
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_ARRAY
        #include <array>            // for std::array
        namespace unboost {
            using std::array;
        }
    #elif defined(UNBOOST_USE_TR1_ARRAY)
        #ifdef _MSC_VER
            #include <array>        // for std::tr1::array
        #else
            #include <tr1/array>    // for std::tr1::array
        #endif
        namespace unboost {
            using std::tr1::array;
        }
    #elif defined(UNBOOST_USE_BOOST_ARRAY)
        #include <boost/array.hpp>  // for boost::array
        namespace unboost {
            using boost::array;
        }
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif

//////////////////////////////////////////////////////////////////////////////
// regular expression (regex)

#ifdef UNBOOST_USE_REGEX
    // If not choosed, choose one
    #if ((defined(UNBOOST_USE_CXX11_REGEX) + defined(UNBOOST_USE_BOOST_REGEX)) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_REGEX
        #elif defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_REGEX
        #else
            #ifdef UNBOOST_CXX11    // C++11
                #define UNBOOST_USE_CXX11_REGEX
            #elif defined(_MSC_VER)
                #if (_MSC_VER >= 1600)
                    // Visual C++ 2010 and later
                    #define UNBOOST_USE_CXX11_REGEX
                #else
                    #define UNBOOST_USE_BOOST_REGEX
                #endif
            #else
                #define UNBOOST_USE_BOOST_REGEX
            #endif
        #endif
    #endif
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_REGEX
        #include <regex>
        namespace unboost {
            using std::regex;
            using std::regex_match;
            using std::regex_search;
            using std::regex_replace;
            using std::cmatch;
            using std::wcmatch;
            using std::smatch;
            using std::wsmatch;
            using std::regex_iterator;
            using std::cregex_iterator;
            using std::wcregex_iterator;
            using std::sregex_iterator;
            using std::wsregex_iterator;
            using std::regex_traits;
            using std::regex_error;
            using std::regex_token_iterator;
            namespace regex_constants {
                typedef std::regex_constants::error_type error_type;
                static constexpr error_type error_collate = std::regex_constants::error_collate;
                static constexpr error_type error_ctype = std::regex_constants::error_ctype;
                static constexpr error_type error_escape = std::regex_constants::error_escape;
                static constexpr error_type error_backref = std::regex_constants::error_backref;
                static constexpr error_type error_brack = std::regex_constants::error_brack;
                static constexpr error_type error_paren = std::regex_constants::error_paren;
                static constexpr error_type error_brace = std::regex_constants::error_brace;
                static constexpr error_type error_badbrace = std::regex_constants::error_badbrace;
                static constexpr error_type error_range = std::regex_constants::error_range;
                static constexpr error_type error_space = std::regex_constants::error_space;
                static constexpr error_type error_badrepeat = std::regex_constants::error_badrepeat;
                static constexpr error_type error_complexity = std::regex_constants::error_complexity;
                static constexpr error_type error_stack = std::regex_constants::error_stack;

                typedef std::regex_constants::match_flag_type match_flag_type;
                static constexpr match_flag_type match_default = std::regex_constants::match_default;
                static constexpr match_flag_type match_not_bol = std::regex_constants::match_not_bol;
                static constexpr match_flag_type match_not_eol = std::regex_constants::match_not_eol;
                static constexpr match_flag_type match_not_bow = std::regex_constants::match_not_bow;
                static constexpr match_flag_type match_not_eow = std::regex_constants::match_not_eow;
                static constexpr match_flag_type match_any = std::regex_constants::match_any;
                static constexpr match_flag_type match_not_null = std::regex_constants::match_not_null;
                static constexpr match_flag_type match_continuous = std::regex_constants::match_continuous;
                static constexpr match_flag_type match_prev_avail = std::regex_constants::match_prev_avail;
                static constexpr match_flag_type format_default = std::regex_constants::format_default;
                static constexpr match_flag_type format_sed = std::regex_constants::format_sed;
                static constexpr match_flag_type format_no_copy = std::regex_constants::format_no_copy;
                static constexpr match_flag_type format_first_only = std::regex_constants::format_first_only;

                typedef std::regex_constants::syntax_option_type syntax_option_type;
                static constexpr syntax_option_type icase = std::regex_constants::icase;
                static constexpr syntax_option_type nosubs = std::regex_constants::nosubs;
                static constexpr syntax_option_type optimize = std::regex_constants::optimize;
                static constexpr syntax_option_type collate = std::regex_constants::collate;
                static constexpr syntax_option_type ECMAScript = std::regex_constants::ECMAScript;
                static constexpr syntax_option_type basic = std::regex_constants::basic;
                static constexpr syntax_option_type extended = std::regex_constants::extended;
                static constexpr syntax_option_type awk = std::regex_constants::awk;
                static constexpr syntax_option_type grep = std::regex_constants::grep;
                static constexpr syntax_option_type egrep = std::regex_constants::egrep;
            } // namespace regex_constants
        } // namespace unboost
    #elif defined(UNBOOST_USE_BOOST_REGEX)
        #include <boost/regex.hpp>
        namespace unboost {
            using boost::regex;
            using boost::regex_match;
            using boost::regex_search;
            using boost::regex_replace;
            using boost::cmatch;
            using boost::wcmatch;
            using boost::smatch;
            using boost::wsmatch;
            using boost::regex_iterator;
            using boost::cregex_iterator;
            using boost::wcregex_iterator;
            using boost::sregex_iterator;
            using boost::wsregex_iterator;
            using boost::regex_traits;
            using boost::regex_error;
            using boost::regex_token_iterator;
            namespace regex_constants {
                typedef boost::regex_constants::error_type error_type;
                static const error_type error_collate = boost::regex_constants::error_collate;
                static const error_type error_ctype = boost::regex_constants::error_ctype;
                static const error_type error_escape = boost::regex_constants::error_escape;
                static const error_type error_backref = boost::regex_constants::error_backref;
                static const error_type error_brack = boost::regex_constants::error_brack;
                static const error_type error_paren = boost::regex_constants::error_paren;
                static const error_type error_brace = boost::regex_constants::error_brace;
                static const error_type error_badbrace = boost::regex_constants::error_badbrace;
                static const error_type error_range = boost::regex_constants::error_range;
                static const error_type error_space = boost::regex_constants::error_space;
                static const error_type error_badrepeat = boost::regex_constants::error_badrepeat;
                static const error_type error_complexity = boost::regex_constants::error_complexity;
                static const error_type error_stack = boost::regex_constants::error_stack;

                typedef boost::regex_constants::match_flag_type match_flag_type;
                static const match_flag_type match_default = boost::regex_constants::match_default;
                static const match_flag_type match_not_bol = boost::regex_constants::match_not_bol;
                static const match_flag_type match_not_eol = boost::regex_constants::match_not_eol;
                static const match_flag_type match_not_bow = boost::regex_constants::match_not_bow;
                static const match_flag_type match_not_eow = boost::regex_constants::match_not_eow;
                static const match_flag_type match_any = boost::regex_constants::match_any;
                static const match_flag_type match_not_null = boost::regex_constants::match_not_null;
                static const match_flag_type match_continuous = boost::regex_constants::match_continuous;
                static const match_flag_type match_prev_avail = boost::regex_constants::match_prev_avail;
                static const match_flag_type format_default = boost::regex_constants::format_default;
                static const match_flag_type format_sed = boost::regex_constants::format_sed;
                static const match_flag_type format_no_copy = boost::regex_constants::format_no_copy;
                static const match_flag_type format_first_only = boost::regex_constants::format_first_only;

                typedef boost::regex_constants::syntax_option_type syntax_option_type;
                static const syntax_option_type icase = boost::regex_constants::icase;
                static const syntax_option_type nosubs = boost::regex_constants::nosubs;
                static const syntax_option_type optimize = boost::regex_constants::optimize;
                static const syntax_option_type collate = boost::regex_constants::collate;
                static const syntax_option_type ECMAScript = boost::regex_constants::ECMAScript;
                static const syntax_option_type basic = boost::regex_constants::basic;
                static const syntax_option_type extended = boost::regex_constants::extended;
                static const syntax_option_type awk = boost::regex_constants::awk;
                static const syntax_option_type grep = boost::regex_constants::grep;
                static const syntax_option_type egrep = boost::regex_constants::egrep;
            } // namespace regex_constants
        } // namespace unboost
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif  // def UNBOOST_USE_REGEX

//////////////////////////////////////////////////////////////////////////////
// conversion between number and string

#ifdef UNBOOST_USE_CONVERSION
    // If not choosed, choose one
    #if ((defined(UNBOOST_USE_CXX11_CONVERSION) + defined(UNBOOST_USE_BOOST_CONVERSION)) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_CONVERSION
        #elif defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_CONVERSION
        #else
            #ifdef UNBOOST_CXX11    // C++11
                #define UNBOOST_USE_CXX11_CONVERSION
            #elif defined(_MSC_VER)
                #if (_MSC_VER >= 1600)
                    // Visual C++ 2010 and later
                    #define UNBOOST_USE_CXX11_CONVERSION
                #else
                    #define UNBOOST_USE_BOOST_CONVERSION
                #endif
            #else
                #define UNBOOST_USE_BOOST_CONVERSION
            #endif
        #endif
    #endif
    #include <cstdlib>
    #include <string>       // for std::string, std::wstring
    #include <sstream>      // for std::stringstream
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_CONVERSION
        #include <typeinfo> // for std::bad_cast
        namespace unboost {
            class bad_lexical_cast : public std::bad_cast {
            public:
                bad_lexical_cast() {}
            };
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
        #include <boost/exception/to_string.hpp>    // for boost::to_string
        #include <climits>      // for INT_MAX, INT_MIN, FLT_MAX, ...
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
                    throw std::invalid_argument("stol");
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
                    throw std::invalid_argument("stoul");
                }
                return ret;
            }
            inline int stoi(const std::string& str, size_t *pos = NULL, int base = 10) {
                long n = unboost::stol(str, pos, base);
                if (n > INT_MAX || n < INT_MIN) {
                    throw std::out_of_range("stoi");
                }
                return static_cast<int>(n);
            }
            #ifdef UNBOOST_CXX11    // C++11
                inline long long
                stoll(const std::string& str, size_t *pos = NULL, int base = 10) {
                    long long ret;
                    size_t npos;
                    if (pos == NULL) {
                        pos = &npos;
                    }
                    char *end = NULL;
                    ret = std::strtoll(str.c_str(), &end, base);
                    *pos = end - str.c_str();
                    if (*pos == 0) {
                        throw std::invalid_argument("stoll");
                    }
                    return ret;
                }
                inline unsigned long long
                stoull(const std::string& str, size_t *pos = NULL, int base = 10) {
                    unsigned long long ret;
                    size_t npos;
                    if (pos == NULL) {
                        pos = &npos;
                    }
                    char *end = NULL;
                    ret = std::strtoull(str.c_str(), &end, base);
                    *pos = end - str.c_str();
                    if (*pos == 0) {
                        throw std::invalid_argument("stoull");
                    }
                    return ret;
                }
            #else   // ndef UNBOOST_CXX11
                inline __int64 stoll(const std::string& str) {
                    // TODO: support pos and base
                    std::stringstream stream;
                    stream << str;
                    __int64 result;
                    stream >> result;
                    if (stream.fail()) {
                        throw std::invalid_argument("stoll");
                    }
                    return result;
                }
                inline unsigned __int64 stoull(const std::string& str) {
                    // TODO: support pos and base
                    std::stringstream stream;
                    stream << str;
                    unsigned __int64 result;
                    stream >> result;
                    if (stream.fail()) {
                        throw std::invalid_argument("stoull");
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
                    throw std::invalid_argument("stof");
                }
                if (d > FLT_MAX || d < -FLT_MAX) {
                    throw std::out_of_range("stof");
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
                    throw std::invalid_argument("stod");
                }
                return ret;
            }
            using boost::to_string;
            template <typename T>
            inline std::wstring to_wstring(const T& value) {
                std::wstringstream stream;
                stream << value;
                return stream.str();
            }
        } // namespace unboost
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif  // def UNBOOST_USE_CONVERSION

//////////////////////////////////////////////////////////////////////////////
// math functions

#ifdef UNBOOST_USE_COMPLEX_FUNCTIONS
    // If not choosed, choose one
    #if ((defined(UNBOOST_USE_CXX11_COMPLEX_FUNCTIONS) + defined(UNBOOST_USE_BOOST_COMPLEX_FUNCTIONS)) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_COMPLEX_FUNCTIONS
        #elif defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_COMPLEX_FUNCTIONS
        #else
            #ifdef UNBOOST_CXX11    // C++11
                #define UNBOOST_USE_CXX11_COMPLEX_FUNCTIONS
            #else
                #define UNBOOST_USE_BOOST_COMPLEX_FUNCTIONS
            #endif
        #endif
    #endif
    #include <cmath>
    #include <complex>
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_COMPLEX_FUNCTIONS
        namespace unboost {
            namespace math {
                using std::complex;
                using std::fabs;
                using std::asin;
                using std::acos;
                using std::atan;
                using std::asinh;
                using std::acosh;
                using std::atanh;
            } // namespace math
        } // namespace unboost
    #elif defined(UNBOOST_USE_BOOST_COMPLEX_FUNCTIONS)
        #include <boost/math/complex.hpp>
        namespace unboost {
            namespace math {
                using boost::math::fabs;
                using boost::math::asin;
                using boost::math::acos;
                using boost::math::atan;
                using boost::math::asinh;
                using boost::math::acosh;
                using boost::math::atanh;
            } // namespace math
        } // namespace unboost
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif  // def UNBOOST_USE_COMPLEX_FUNCTIONS

//////////////////////////////////////////////////////////////////////////////
// random generation

#ifdef UNBOOST_USE_RANDOM
    // If not choosed, choose one
    #if ((defined(UNBOOST_USE_CXX11_RANDOM) + defined(UNBOOST_USE_BOOST_RANDOM)) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_RANDOM
        #elif defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_RANDOM
        #else
            #ifdef UNBOOST_CXX11    // C++11
                #define UNBOOST_USE_CXX11_RANDOM
            #elif defined(_MSC_VER)
                #if (_MSC_VER >= 1600)
                    // Visual C++ 2010 and later
                    #define UNBOOST_USE_CXX11_RANDOM
                #else
                    #define UNBOOST_USE_BOOST_RANDOM
                #endif
            #else
                #define UNBOOST_USE_BOOST_RANDOM
            #endif
        #endif
    #endif
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_RANDOM
        #include <random>
        namespace unboost {
            namespace random {
                using std::random_device;
                using std::minstd_rand0;
                using std::minstd_rand;
                using std::mt19937;
                using std::mt19937_64;
                using std::ranlux24_base;
                using std::ranlux48_base;
                using std::ranlux24;
                using std::ranlux48;
                using std::knuth_b;
                using std::uniform_int_distribution;
                using std::uniform_real_distribution;
                using std::generate_canonical;
                using std::bernoulli_distribution;
                using std::binomial_distribution;
                using std::negative_binomial_distribution;
                using std::geometric_distribution;
                using std::poisson_distribution;
                using std::exponential_distribution;
                using std::gamma_distribution;
                using std::weibull_distribution;
                using std::extreme_value_distribution;
                using std::normal_distribution;
                using std::lognormal_distribution;
                using std::chi_squared_distribution;
                using std::cauchy_distribution;
                using std::fisher_f_distribution;
                using std::student_t_distribution;
                using std::discrete_distribution;
                using std::piecewise_constant_distribution;
                using std::piecewise_linear_distribution;
                using std::seed_seq;
            } // namespace random
            using namespace random;
        } // namespace unboost
    #elif defined(UNBOOST_USE_BOOST_RANDOM)
        #include <boost/random/random_device.hpp>
        #include <boost/random.hpp>
        namespace unboost {
            namespace random {
                using boost::random::random_device;
                using boost::random::minstd_rand0;
                using boost::random::minstd_rand;
                using boost::random::mt19937;
                using boost::random::mt19937_64;
                using boost::random::ranlux24_base;
                using boost::random::ranlux48_base;
                using boost::random::ranlux24;
                using boost::random::ranlux48;
                using boost::random::knuth_b;
                using boost::random::uniform_int_distribution;
                using boost::random::uniform_real_distribution;
                using boost::random::generate_canonical;
                using boost::random::bernoulli_distribution;
                using boost::random::binomial_distribution;
                using boost::random::negative_binomial_distribution;
                using boost::random::geometric_distribution;
                using boost::random::poisson_distribution;
                using boost::random::exponential_distribution;
                using boost::random::gamma_distribution;
                using boost::random::weibull_distribution;
                using boost::random::extreme_value_distribution;
                using boost::random::normal_distribution;
                using boost::random::lognormal_distribution;
                using boost::random::chi_squared_distribution;
                using boost::random::cauchy_distribution;
                using boost::random::fisher_f_distribution;
                using boost::random::student_t_distribution;
                using boost::random::discrete_distribution;
                using boost::random::piecewise_constant_distribution;
                using boost::random::piecewise_linear_distribution;
                using boost::random::seed_seq;
            } // namespace random
            using namespace random;
        } // namespace unboost
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif  // def UNBOOST_USE_RANDOM

//////////////////////////////////////////////////////////////////////////////
// chrono

#ifdef UNBOOST_USE_CHRONO
    // If not choosed, choose one
    #if ((defined(UNBOOST_USE_CXX11_CHRONO) + defined(UNBOOST_USE_BOOST_CHRONO)) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_CHRONO
        #elif defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_CHRONO
        #else
            #ifdef UNBOOST_CXX11    // C++11
                #define UNBOOST_USE_CXX11_CHRONO
            #elif defined(_MSC_VER)
                #if (_MSC_VER >= 1700)
                    // Visual C++ 2012 and later
                    #define UNBOOST_USE_CXX11_CHRONO
                #else
                    #define UNBOOST_USE_BOOST_CHRONO
                #endif
            #else
                #define UNBOOST_USE_BOOST_CHRONO
            #endif
        #endif
    #endif
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_CHRONO
        #include <chrono>
        namespace unboost {
            namespace chrono {
                using std::chrono::duration;
                using std::chrono::duration_cast;
                using std::chrono::system_clock;
                using std::chrono::steady_clock;
                using std::chrono::high_resolution_clock;
                using std::chrono::time_point;
                using std::chrono::treat_as_floating_point;
                using std::chrono::duration_values;
                using std::chrono::nanoseconds;
                using std::chrono::microseconds;
                using std::chrono::milliseconds;
                using std::chrono::seconds;
                using std::chrono::minutes;
                using std::chrono::hours;
                using std::chrono::time_point_cast;
            } // namespace chrono
        } // namespace unboost
    #elif defined(UNBOOST_USE_BOOST_CHRONO)
        #include <boost/chrono/include.hpp>
        namespace unboost {
            namespace chrono {
                using boost::chrono::duration;
                using boost::chrono::duration_cast;
                using boost::chrono::system_clock;
                using boost::chrono::steady_clock;
                using boost::chrono::high_resolution_clock;
                using boost::chrono::time_point;
                using boost::chrono::treat_as_floating_point;
                using boost::chrono::duration_values;
                using boost::chrono::nanoseconds;
                using boost::chrono::microseconds;
                using boost::chrono::milliseconds;
                using boost::chrono::seconds;
                using boost::chrono::minutes;
                using boost::chrono::hours;
                using boost::chrono::time_point_cast;
            } // namespace chrono
        } // namespace unboost
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif  // def UNBOOST_USE_CHRONO

//////////////////////////////////////////////////////////////////////////////
// unordered set

#ifdef UNBOOST_USE_UNORDERED_SET
    // If not choosed, choose one
    #if ((defined(UNBOOST_USE_CXX11_UNORDERED_SET) + defined(UNBOOST_USE_TR1_UNORDERED_SET) + defined(UNBOOST_USE_BOOST_UNORDERED_SET)) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_UNORDERED_SET
        #elif defined(UNBOOST_USE_TR1)
            #define UNBOOST_USE_TR1_UNORDERED_SET
        #else
            #if defined(_MSC_VER)
                #if (_MSC_VER >= 1600)
                    // Visual C++ 2010 and later
                    #define UNBOOST_USE_CXX11_UNORDERED_SET
                #elif (_MSC_VER >= 1500)
                    // Visual C++ 2008
                    #define UNBOOST_USE_TR1_UNORDERED_SET
                #else
                    #define UNBOOST_USE_BOOST_UNORDERED_SET
                #endif
            #elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
                // GCC 4.3 and later
                #define UNBOOST_USE_TR1_UNORDERED_SET
            #else
                #define UNBOOST_USE_BOOST_UNORDERED_SET
            #endif
        #endif
    #endif
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_UNORDERED_SET
        #include <unordered_set>            // for std::unordered_set, ...
        namespace unboost {
            using std::unordered_set;
            using std::unordered_multiset;
        }
    #elif defined(UNBOOST_USE_TR1_UNORDERED_SET)
        #ifdef _MSC_VER
            #include <unordered_set>
        #else
            #include <tr1/unordered_set>    // for std::tr1::unordered_set, ...
        #endif
        namespace unboost {
            using std::tr1::unordered_set;
            using std::tr1::unordered_multiset;
        }
    #elif defined(UNBOOST_USE_BOOST_UNORDERED_SET)
        #include <boost/unordered_set.hpp>  // for boost::unordered_set, ...
        namespace unboost {
            using boost::unordered_set;
            using boost::unordered_multiset;
        }
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif  // def UNBOOST_USE_UNORDERED_SET

//////////////////////////////////////////////////////////////////////////////
// unordered map

#ifdef UNBOOST_USE_UNORDERED_MAP
    // If not choosed, choose one
    #if ((defined(UNBOOST_USE_CXX11_UNORDERED_MAP) + defined(UNBOOST_USE_TR1_UNORDERED_MAP) + defined(UNBOOST_USE_BOOST_UNORDERED_MAP)) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_UNORDERED_MAP
        #elif defined(UNBOOST_USE_TR1)
            #define UNBOOST_USE_TR1_UNORDERED_MAP
        #else
            #if defined(_MSC_VER)
                #if (_MSC_VER >= 1600)
                    // Visual C++ 2010 and later
                    #define UNBOOST_USE_CXX11_UNORDERED_MAP
                #elif (_MSC_VER >= 1500)
                    // Visual C++ 2008
                    #define UNBOOST_USE_TR1_UNORDERED_MAP
                #else
                    #define UNBOOST_USE_BOOST_UNORDERED_MAP
                #endif
            #elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
                // GCC 4.3 and later
                #define UNBOOST_USE_TR1_UNORDERED_MAP
            #else
                #define UNBOOST_USE_BOOST_UNORDERED_MAP
            #endif
        #endif
    #endif
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_UNORDERED_MAP
        #include <unordered_map>            // for std::unordered_map, ...
        namespace unboost {
            using std::unordered_map;
            using std::unordered_multimap;
        }
    #elif defined(UNBOOST_USE_TR1_UNORDERED_MAP)
        #ifdef _MSC_VER
            #include <unordered_map>        // for std::tr1::unordered_map, ...
        #else
            #include <tr1/unordered_map>    // for std::tr1::unordered_map, ...
        #endif
        namespace unboost {
            using std::tr1::unordered_map;
            using std::tr1::unordered_multimap;
        }
    #elif defined(UNBOOST_USE_BOOST_UNORDERED_MAP)
        #include <boost/unordered_map.hpp>  // for boost::unordered_map, ...
        namespace unboost {
            using boost::unordered_map;
            using boost::unordered_multimap;
        }
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif  // def UNBOOST_USE_UNORDERED_MAP

//////////////////////////////////////////////////////////////////////////////
// foreach

#ifdef UNBOOST_USE_FOREACH
    // If not choosed, choose one
    #if ((defined(UNBOOST_USE_CXX11_FOREACH) + defined(UNBOOST_USE_BOOST_FOREACH)) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_FOREACH
        #elif defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_FOREACH
        #else
            #ifdef UNBOOST_CXX11    // C++11
                #define UNBOOST_USE_CXX11_FOREACH
            #else
                #define UNBOOST_USE_BOOST_FOREACH
            #endif
        #endif
    #endif
    // Adapt choosed one
    #ifdef UNBOOST_USE_CXX11_FOREACH
        #define UNBOOST_FOREACH(x,y) for (x : y)
    #elif defined(UNBOOST_USE_BOOST_FOREACH)
        #include <boost/foreach.hpp>
        #define UNBOOST_FOREACH BOOST_FOREACH
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif  // def UNBOOST_USE_FOREACH

//////////////////////////////////////////////////////////////////////////////
// tuple

#ifdef UNBOOST_USE_TUPLE
    #if ((defined(UNBOOST_USE_CXX11_TUPLE) + defined(UNBOOST_USE_BOOST_TUPLE)) == 0)
        #ifdef UNBOOST_USE_CXX11
            #define UNBOOST_USE_CXX11_TUPLE
        #elif defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_TUPLE
        #else
            #ifdef UNBOOST_CXX11
                #define UNBOOST_USE_CXX11_TUPLE
            #else
                #define UNBOOST_USE_BOOST_TUPLE
            #endif
        #endif
    #endif
    #ifdef UNBOOST_USE_CXX11_TUPLE
        #include <tuple>
        namespace unboost {
            using std::tuple;
            using std::make_tuple;
            using std::tie;
            using std::get;
            using std::tuple_size;
            using std::ref;
            using std::cref;
            using std::ignore;
            namespace tuples {
                using std::ignore;
                template <typename T>
                struct length {
                    static constexpr size_t value = std::tuple_size<T>::value;
                };
            } // namespace tuples
            using std::tuple_cat;
        } // namespace unboost
    #elif defined(UNBOOST_USE_BOOST_TUPLE)
        #include <boost/tuple/tuple.hpp>
        #include <boost/tuple/tuple_comparison.hpp>
        #include <boost/tuple/tuple_io.hpp>
        namespace unboost {
            using boost::tuple;
            using boost::make_tuple;
            using boost::tie;
            using boost::get;
            template <typename T>
            struct tuple_size {
                static const size_t value = boost::tuples::length<T>::value;
            };
            using boost::ref;
            using boost::cref;
            using boost::tuples::ignore;
            namespace tuples {
                using boost::tuples::ignore;
                using boost::tuples::length;
            } // namespace tuples
            // NOTE: There is no boost::tuple_cat.
        } // namespace unboost
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif  // def UNBOOST_USE_TUPLE

//////////////////////////////////////////////////////////////////////////////
// string algorithm

#ifdef UNBOOST_USE_STRING_ALGORITHM
    #if ((defined(UNBOOST_USE_CXX_STRING_ALGORITHM) + defined(UNBOOST_USE_BOOST_STRING_ALGORITHM)) == 0)
        #if defined(UNBOOST_USE_BOOST)
            #define UNBOOST_USE_BOOST_STRING_ALGORITHM
        #else
            #define UNBOOST_USE_CXX_STRING_ALGORITHM
        #endif
    #endif
    #ifdef UNBOOST_USE_BOOST_STRING_ALGORITHM
        #include <boost/algorithm/string.hpp>
        namespace unboost {
            using boost::is_space;
            using boost::is_alpha;
            using boost::is_alnum;
            using boost::is_digit;
            using boost::is_xdigit;
            using boost::is_lower;
            using boost::is_upper;
            using boost::is_from_range;
            using boost::is_cntrl;
            using boost::is_punct;
            using boost::is_any_of;
            using boost::to_upper;
            using boost::to_lower;
            using boost::trim;
            using boost::trim_left;
            using boost::trim_right;
            using boost::trim_if;
            using boost::trim_left_if;
            using boost::trim_right_if;
            using boost::to_upper_copy;
            using boost::to_lower_copy;
            using boost::trim_copy;
            using boost::trim_left_copy;
            using boost::trim_right_copy;
            using boost::trim_copy_if;
            using boost::trim_left_copy_if;
            using boost::trim_right_copy_if;
            using boost::split;
            using boost::join;
            using boost::replace_all;
            using boost::replace_all_copy;
        } // namespace unboost
    #elif defined(UNBOOST_USE_CXX_STRING_ALGORITHM)
        #include <cctype>
        namespace unboost {
            // TODO: be locale aware
            namespace char_set {
                // ansi
                static const char *spaces = " \t\n\r\f\v";
                static const char *alphas = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
                static const char *alnums = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
                static const char *digits = "0123456789";
                static const char *xdigits = "0123456789ABCDEFabcdef";
                static const char *uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
                static const char *lowers = "abcdefghijklmnopqrstuvwxyz";
                static const char *puncts = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
                // wide
                static const wchar_t *wspaces = L" \t\n\r\f\v\u3000";
                static const wchar_t *walphas = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
                static const wchar_t *walnums = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
                static const wchar_t *wdigits = L"0123456789";
                static const wchar_t *wxdigits = L"0123456789ABCDEFabcdef";
                static const wchar_t *wuppers = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
                static const wchar_t *wlowers = L"abcdefghijklmnopqrstuvwxyz";
                static const wchar_t *wpuncts = L"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
            } // namespace char_set
            struct char_set_predicate {
                std::string     m_char_set;
                std::wstring    m_wchar_set;
                char_set_predicate() {}
                char_set_predicate(const std::string& str) : m_char_set(str) {
                    for (size_t i = 0; i < str.size(); ++i) {
                        m_wchar_set += wchar_t(str[i]);
                    }
                }
                char_set_predicate(const std::wstring& str) : m_wchar_set(str) {
                    for (size_t i = 0; i < str.size(); ++i) {
                        if (str[i] <= 0x7F) {
                            m_char_set += char(str[i]);
                        }
                    }
                }
                char_set_predicate(const std::string& str, const std::wstring& wstr)
                    : m_char_set(str), m_wchar_set(wstr) {}
            }; // struct char_set_predicate
            struct is_space : public char_set_predicate {
                is_space() : char_set_predicate(char_set::spaces, char_set::wspaces) {}
            };
            struct is_alpha : public char_set_predicate {
                is_alpha() : char_set_predicate(char_set::alphas, char_set::walphas) {}
            };
            struct is_alnum : public char_set_predicate {
                is_alnum() : char_set_predicate(char_set::alnums, char_set::walnums) {}
            };
            struct is_digit : public char_set_predicate {
                is_digit() : char_set_predicate(char_set::digits, char_set::wdigits) {}
            };
            struct is_xdigit : public char_set_predicate {
                is_xdigit() : char_set_predicate(char_set::xdigits, char_set::wxdigits) {}
            };
            struct is_lower : public char_set_predicate {
                is_lower() : char_set_predicate(char_set::lowers, char_set::wlowers) {}
            };
            struct is_upper : public char_set_predicate {
                is_upper() : char_set_predicate(char_set::uppers, char_set::wuppers) {}
            };
            struct is_from_range : public char_set_predicate {
                is_from_range(char from, char to) {
                    for (int ch = from; ch <= to; ++ch) {
                        m_char_set += char(ch);
                        m_wchar_set += wchar_t(ch);
                    }
                }
            };
            struct is_cntrl : public is_from_range {
                is_cntrl(char from, char to) : is_from_range(0, '\x1F') {
                    m_char_set += '\x7F';
                    m_wchar_set += static_cast<wchar_t>(0x7F);
                }
            };
            struct is_punct : public char_set_predicate {
                is_punct() : char_set_predicate(char_set::puncts, char_set::wpuncts) {}
            };
            struct is_any_of : public char_set_predicate {
                is_any_of(const std::string& str) : char_set_predicate(str) {}
                is_any_of(const std::wstring& str) : char_set_predicate(str) {}
            };
            inline void to_upper(std::string& str) {
                using namespace std;
                const size_t count = str.size();
                for (size_t i = 0; i < count; ++i) {
                    char& ch = str[i];
                    if (islower(ch)) {
                        ch = toupper(ch);
                    }
                }
            }
            inline void to_lower(std::string& str) {
                using namespace std;
                const size_t count = str.size();
                for (size_t i = 0; i < count; ++i) {
                    char& ch = str[i];
                    if (isupper(ch)) {
                        ch = tolower(ch);
                    }
                }
            }
            inline void to_upper(std::wstring& str) {
                using namespace std;
                const size_t count = str.size();
                for (size_t i = 0; i < count; ++i) {
                    wchar_t& ch = str[i];
                    if (iswlower(ch)) {
                        ch = towupper(ch);
                    }
                }
            }
            inline void to_lower(std::wstring& str) {
                using namespace std;
                const size_t count = str.size();
                for (size_t i = 0; i < count; ++i) {
                    wchar_t& ch = str[i];
                    if (iswupper(ch)) {
                        ch = towlower(ch);
                    }
                }
            }
            inline void trim_if(std::string& str, const char_set_predicate& pred) {
                size_t i = str.find_first_not_of(pred.m_char_set);
                size_t j = str.find_last_not_of(pred.m_char_set);
                if ((i == std::string::npos) || (j == std::string::npos)) {
                    str.clear();
                } else {
                    str = str.substr(i, j - i + 1);
                }
            }
            inline void trim_left_if(std::string& str, const char_set_predicate& pred) {
                size_t i = str.find_first_not_of(pred.m_char_set);
                if (i == std::string::npos) {
                    str.clear();
                } else {
                    str = str.substr(i);
                }
            }
            inline void trim_right_if(std::string& str, const char_set_predicate& pred) {
                size_t j = str.find_last_not_of(pred.m_char_set);
                if (j == std::string::npos) {
                    str.clear();
                } else {
                    str = str.substr(0, j + 1);
                }
            }
            inline void trim(std::string& str) {
                trim_if(str, is_space());
            }
            inline void trim_left(std::string& str) {
                trim_left_if(str, is_space());
            }
            inline void trim_right(std::string& str, const char_set_predicate& pred) {
                trim_right_if(str, is_space());
            }
            inline void trim_if(std::wstring& str, const char_set_predicate& pred) {
                size_t i = str.find_first_not_of(pred.m_wchar_set);
                size_t j = str.find_last_not_of(pred.m_wchar_set);
                if ((i == std::wstring::npos) || (j == std::wstring::npos)) {
                    str.clear();
                } else {
                    str = str.substr(i, j - i + 1);
                }
            }
            inline void trim_left_if(std::wstring& str, const char_set_predicate& pred) {
                size_t i = str.find_first_not_of(pred.m_wchar_set);
                if (i == std::wstring::npos) {
                    str.clear();
                } else {
                    str = str.substr(i);
                }
            }
            inline void trim_right_if(std::wstring& str, const char_set_predicate& pred) {
                size_t j = str.find_last_not_of(pred.m_wchar_set);
                if (j == std::wstring::npos) {
                    str.clear();
                } else {
                    str = str.substr(0, j + 1);
                }
            }
            inline void trim(std::wstring& str) {
                trim_if(str, is_space());
            }
            inline void trim_left(std::wstring& str) {
                trim_left_if(str, is_space());
            }
            inline void trim_right(std::wstring& str) {
                trim_right_if(str, is_space());
            }
            template <typename T_STRING>
            inline T_STRING to_upper_copy(const T_STRING& str) {
                T_STRING copy(str);
                to_upper(copy);
                return copy;
            }
            template <typename T_STRING>
            inline T_STRING to_lower_copy(const T_STRING& str) {
                T_STRING copy(str);
                to_lower(copy);
                return copy;
            }
            template <typename T_STRING>
            inline T_STRING trim_copy(const T_STRING& str) {
                T_STRING copy(str);
                trim(copy);
                return copy;
            }
            template <typename T_STRING>
            inline T_STRING trim_left_copy(const T_STRING& str) {
                T_STRING copy(str);
                trim_left(copy);
                return copy;
            }
            template <typename T_STRING>
            inline T_STRING trim_right_copy(const T_STRING& str) {
                T_STRING copy(str);
                trim_right(copy);
                return copy;
            }
            template <typename T_STRING_CONTAINER>
            void split(T_STRING_CONTAINER& container,
                       const std::string& str,
                       const char_set_predicate& pred)
            {
                container.clear();
                size_t i = 0, j = str.find_first_of(pred.m_char_set);
                while (j != T_STRING_CONTAINER::value_type::npos) {
                    container.push_back(str.substr(i, j - i));
                    i = j + 1;
                    j = str.find_first_of(pred.m_char_set, i);
                }
                container.push_back(str.substr(i));
            }
            template <typename T_STRING_CONTAINER>
            void split(T_STRING_CONTAINER& container,
                       const std::wstring& str,
                       const char_set_predicate& pred)
            {
                container.clear();
                size_t i = 0, j = str.find_first_of(pred.m_wchar_set);
                while (j != T_STRING_CONTAINER::value_type::npos) {
                    container.push_back(str.substr(i, j - i));
                    i = j + 1;
                    j = str.find_first_of(pred.m_wchar_set, i);
                }
                container.push_back(str.substr(i));
            }
            template <typename T_STRING_CONTAINER>
            typename T_STRING_CONTAINER::value_type
            join(const T_STRING_CONTAINER& container,
                 const typename T_STRING_CONTAINER::value_type& sep)
            {
                typename T_STRING_CONTAINER::value_type result;
                typename T_STRING_CONTAINER::const_iterator it = container.begin();
                typename T_STRING_CONTAINER::const_iterator end = container.end();
                if (it != end) {
                    result = *it;
                    for (++it; it != end; ++it) {
                        result += sep;
                        result += *it;
                    }
                }
                return result;
            }
            template <typename T_STRING>
            void replace_all(T_STRING& str,
                             const T_STRING& from, const T_STRING& to)
            {
                size_t i = 0;
                for (;;) {
                    i = str.find(from, i);
                    if (i == T_STRING::npos)
                        break;
                    str.replace(i, from.size(), to);
                    i += to.size();
                }
            }
            template <typename T_STRING>
            inline void replace_all(T_STRING& str,
                const typename T_STRING::value_type *from,
                const typename T_STRING::value_type *to)
            {
                replace_all(str, T_STRING(from), T_STRING(to));
            }
            template <typename T_STRING>
            inline T_STRING replace_all_copy(
                const T_STRING& str,
                const T_STRING& from, const T_STRING& to)
            {
                T_STRING copy(str);
                replace_all(copy, from, to);
                return copy;
            }
            template <typename T_STRING>
            inline T_STRING replace_all_copy(const T_STRING& str,
                const typename T_STRING::value_type *from,
                const typename T_STRING::value_type *to)
            {
                T_STRING copy(str);
                replace_all(copy, from, to);
                return copy;
            }
        } // namespace unboost
    #else
        #error Your compiler is not supported yet. You lose.
    #endif
#endif   // def UNBOOST_USE_STRING_ALGORITHM

//////////////////////////////////////////////////////////////////////////////
// assert

#ifdef UNBOOST_USE_ASSERT
    #if ((defined(UNBOOST_USE_CXX_ASSERT) + defined(UNBOOST_USE_BOOST_ASSERT)) == 0)
        #ifdef UNBOOST_USE_BOOST
            #define UNBOOST_USE_BOOST_ASSERT
        #endif
    #endif
    #if defined(UNBOOST_USE_BOOST_ASSERT)
        #ifdef UNBOOST_DISABLE_ASSERTS
            #define BOOST_DISABLE_ASSERTS
        #endif
        #include <boost/assert.hpp>
        #define UNBOOST_ASSERT          BOOST_ASSERT
        #define UNBOOST_ASSERT_MSG      BOOST_ASSERT_MSG
        #define UNBOOST_VERIFY          BOOST_VERIFY
        #define UNBOOST_VERIFY_MSG      BOOST_VERIFY_MSG
        #ifdef BOOST_ASSERT_IS_VOID
            #define UNBOOST_ASSERT_IS_VOID
        #endif
    #else
        #if defined(UNBOOST_DISABLE_ASSERTS)
            #define UNBOOST_ASSERT(exp)             ((void)0)
            #define UNBOOST_ASSERT_MSG(exp,msg)     ((void)0)
            #define UNBOOST_VERIFY(exp)             ((void)(exp))
            #define UNBOOST_VERIFY_MSG(exp,msg)     ((void)(exp))
        #else
            #define UNBOOST_ASSERT(exp)             assert(exp)
            #define UNBOOST_ASSERT_MSG(exp,msg)     assert((exp) && (msg))
            #define UNBOOST_VERIFY                  UNBOOST_ASSERT
            #define UNBOOST_VERIFY_MSG              UNBOOST_ASSERT_MSG
        #endif
        #if defined(NDEBUG)
            #define UNBOOST_ASSERT_IS_VOID
        #endif
    #endif
#endif  // def UNBOOST_USE_ASSERT

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef UNBOOST_HPP_

//////////////////////////////////////////////////////////////////////////////
