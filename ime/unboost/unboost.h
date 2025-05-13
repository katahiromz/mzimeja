/* unboost.h --- Unboost core */
/****************************************************************************/

#ifndef UNBOOST_CORE_H_
#define UNBOOST_CORE_H_

#if (_MSC_VER > 1000)
    #pragma once
#endif

#define UNBOOST_VERSION             40
#define UNBOOST_VERSION_STRING      "Unboost 40 by Katayama Hirofumi MZ"

#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _CRT_NON_CONFORMING_SWPRINTFS
    #define _CRT_NON_CONFORMING_SWPRINTFS
#endif

#ifndef __cplusplus
    #ifndef COBJMACROS
        #define COBJMACROS
    #endif
#endif

#ifdef va_start
    #error You should #include "unboost/unboost.h" before standard C/C++ headers.
#endif
#ifdef _INC_WINDOWS
    #error You should #include "unboost/unboost.h" before <windows.h>.
#endif

#ifdef __cplusplus
    #include <cassert>      // for the assert macro
    #include <string>       // for std::string and std::wstring, ...
    #include <vector>       // for std::vector
    #include <iostream>     // for std::cout, std::endl, ...
    #include <algorithm>    // for std::swap
#else
    #include <assert.h>     /* for the assert macro */
#endif

/****************************************************************************/
/* old compiler supports */

#if defined(__BORLANDC__) && (__BORLANDC__ <= 0x0551)
    #ifndef UNBOOST_OLD_BORLAND
        #define UNBOOST_OLD_BORLAND
    #endif
    #ifndef inline
        #define inline __inline
    #endif
#endif

#ifndef UNBOOST_OLD_COMPILER
    #ifdef UNBOOST_OLD_BORLAND
        #define UNBOOST_OLD_COMPILER
    #endif
    #ifdef __WATCOMC__
        #define UNBOOST_OLD_COMPILER
    #endif
#endif

/****************************************************************************/
/* Is Unboost on C++11? (UNBOOST_CXX11) */

#ifndef UNBOOST_CXX11
    #if (__cplusplus >= 201103L)    /* C++11 */
        #define UNBOOST_CXX11
    #endif
#endif

#ifdef UNBOOST_NO_CXX11
    #undef UNBOOST_CXX11
#endif

#ifndef UNBOOST_NOEXCEPT
    #if (__cplusplus >= 201103L)
        #define UNBOOST_NOEXCEPT noexcept
    #else
        #define UNBOOST_NOEXCEPT
    #endif
#endif

/****************************************************************************/
/* _char16_t, _char32_t, u16string, u32string */

#ifdef __cplusplus
    #if (__cplusplus >= 201103L) || defined(UNBOOST_HAVE_CHARXX_T)
        namespace unboost {
            // NOTE: char16_t, char32_t are keywords on C++11
            typedef char16_t        _char16_t;
            typedef char32_t        _char32_t;
            typedef std::string     u8string;
            typedef std::u16string  u16string;
            typedef std::u32string  u32string;
        } // namespace unboost
        #ifndef UNBOOST_HAVE_CHARXX_T
            #define UNBOOST_HAVE_CHARXX_T
        #endif
    #else
        namespace unboost {
            typedef unsigned short  _char16_t;
            typedef unsigned int    _char32_t;
            typedef std::string                     u8string;
            typedef std::basic_string<_char16_t>    u16string;
            typedef std::basic_string<_char32_t>    u32string;
        } // namespace unboost
    #endif
#endif  /* def __cplusplus */

/****************************************************************************/
/* treat GNU extension */

#ifndef __extension__
    #if !defined(__GNUC__) && !defined(__clang__)
        #define __extension__   /**/
    #endif
#endif

/****************************************************************************/
/* UNBOOST_CONSTEXPR */

#ifdef UNBOOST_CXX11
    #define UNBOOST_CONSTEXPR constexpr
#elif (__cplusplus >= 201103L && !defined(UNBOOST_NO_CXX11))
    #define UNBOOST_CONSTEXPR constexpr
#else
    #define UNBOOST_CONSTEXPR
#endif

/****************************************************************************/
/* dependencies */

/* Unboost's filesystem depends on system-specific system */
#ifdef UNBOOST_USE_UNBOOST_FILESYSTEM
    #ifdef _WIN32
        #ifndef UNBOOST_USE_WIN32_SYSTEM
            #define UNBOOST_USE_WIN32_SYSTEM
        #endif
    #else
        #ifndef UNBOOST_USE_POSIX_SYSTEM
            #define UNBOOST_USE_POSIX_SYSTEM
        #endif
    #endif
#endif

/* thread and chrono depends on each other */
#ifdef UNBOOST_USE_CXX11_THREAD
    #ifndef UNBOOST_USE_CXX11_CHRONO
        #define UNBOOST_USE_CXX11_CHRONO
    #endif
#endif
#ifdef UNBOOST_USE_BOOST_THREAD
    #ifndef UNBOOST_USE_BOOST_CHRONO
        #define UNBOOST_USE_BOOST_CHRONO
    #endif
#endif
#ifdef UNBOOST_USE_WIN32_THREAD
    #ifndef UNBOOST_USE_WIN32_CHRONO
        #define UNBOOST_USE_WIN32_CHRONO
    #endif
#endif
#ifdef UNBOOST_USE_POSIX_THREAD
    #ifndef UNBOOST_USE_POSIX_CHRONO
        #define UNBOOST_USE_POSIX_CHRONO
    #endif
#endif
#ifdef UNBOOST_USE_CXX11_CHRONO
    #ifndef UNBOOST_USE_CXX11_THREAD
        #define UNBOOST_USE_CXX11_THREAD
    #endif
#endif
#ifdef UNBOOST_USE_BOOST_CHRONO
    #ifndef UNBOOST_USE_BOOST_THREAD
        #define UNBOOST_USE_BOOST_THREAD
    #endif
#endif
#ifdef UNBOOST_USE_WIN32_CHRONO
    #ifndef UNBOOST_USE_WIN32_THREAD
        #define UNBOOST_USE_WIN32_THREAD
    #endif
#endif
#ifdef UNBOOST_USE_POSIX_CHRONO
    #ifndef UNBOOST_USE_POSIX_THREAD
        #define UNBOOST_USE_POSIX_THREAD
    #endif
#endif

/* Unboost's chrono depends on Unboost's type_traits and ratio */
#ifdef UNBOOST_USE_UNBOOST_CHRONO
    #ifndef UNBOOST_USE_UNBOOST_RATIO
        #define UNBOOST_USE_UNBOOST_RATIO
    #endif
    #ifndef UNBOOST_USE_UNBOOST_TYPE_TRAITS
        #define UNBOOST_USE_UNBOOST_TYPE_TRAITS
    #endif
#endif

/* Unboost's filesystem depends on Unboost's smart_ptr */
#ifdef UNBOOST_USE_UNBOOST_FILESYSTEM
    #ifndef UNBOOST_USE_UNBOOST_SMART_PTR
        #define UNBOOST_USE_UNBOOST_SMART_PTR
    #endif
#endif

/* Unboost's smart_ptr depends on Unboost's type_traits */
#ifdef UNBOOST_USE_UNBOOST_SMART_PTR
    #ifndef UNBOOST_USE_UNBOOST_TYPE_TRAITS
        #define UNBOOST_USE_UNBOOST_TYPE_TRAITS
    #endif
#endif

/* Unboost's ratio depends on Unboost's static_assert */
#ifdef UNBOOST_USE_UNBOOST_RATIO
    #ifndef UNBOOST_USE_UNBOOST_STATIC_ASSERT
        #define UNBOOST_USE_UNBOOST_STATIC_ASSERT
    #endif
#endif

/* win32/posix thread and win32/posix event depends each other */
#ifdef UNBOOST_USE_WIN32_THREAD
    #ifndef UNBOOST_USE_WIN32_EVENT
        #define UNBOOST_USE_WIN32_EVENT
    #endif
#endif
#ifdef UNBOOST_USE_WIN32_EVENT
    #ifndef UNBOOST_USE_WIN32_THREAD
        #define UNBOOST_USE_WIN32_THREAD
    #endif
#endif
#ifdef UNBOOST_USE_POSIX_THREAD
    #ifndef UNBOOST_USE_POSIX_EVENT
        #define UNBOOST_USE_POSIX_EVENT
    #endif
#endif
#ifdef UNBOOST_USE_POSIX_EVENT
    #ifndef UNBOOST_USE_POSIX_THREAD
        #define UNBOOST_USE_POSIX_THREAD
    #endif
#endif

/* win32/posix thread depends on win32/posix system */
#ifdef UNBOOST_USE_WIN32_THREAD
    #ifndef UNBOOST_USE_WIN32_SYSTEM
        #define UNBOOST_USE_WIN32_SYSTEM
    #endif
#endif
#ifdef UNBOOST_USE_POSIX_THREAD
    #ifndef UNBOOST_USE_POSIX_SYSTEM
        #define UNBOOST_USE_POSIX_SYSTEM
    #endif
#endif

/* Unboost's type_traits depends on Unboost's rvref */
#ifdef UNBOOST_USE_UNBOOST_TYPE_TRAITS
    #ifndef UNBOOST_USE_UNBOOST_RV_REF
        #define UNBOOST_USE_UNBOOST_RV_REF
    #endif
#endif

/* Unboost's unordered_set depends on Unboost's forward_list */
#ifdef UNBOOST_USE_UNBOOST_UNORDERED_SET
    #ifndef UNBOOST_USE_UNBOOST_FORWARD_LIST
        #define UNBOOST_USE_UNBOOST_FORWARD_LIST
    #endif
#endif

/****************************************************************************/
/* consistency checks */

#if ((defined(UNBOOST_USE_CXX11) + defined(UNBOOST_USE_TR1) + defined(UNBOOST_USE_BOOST)) >= 2)
    #error Choose one or none of UNBOOST_USE_CXX11, UNBOOST_USE_TR1 and UNBOOST_USE_BOOST. You lose.
#endif

/****************************************************************************/
/* swapping */

#ifdef __cplusplus
    namespace unboost {
        using std::swap;
    } // namespace unboost
#endif  /* def __cplusplus */

/****************************************************************************/
/* 64-bit integer */

#ifdef _WIN32
    #ifndef _INC_WINDOWS
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif
        #include <windows.h>
    #endif
#endif

#ifdef __cplusplus
namespace unboost {
#endif
    #ifdef _WIN32
        typedef LONGLONG            _int64_t;
        typedef DWORDLONG           _uint64_t;
    #else
        typedef long long           _int64_t;
        typedef unsigned long long  _uint64_t;
    #endif
#ifdef __cplusplus
} // namespace unboost
#endif

/****************************************************************************/
/* _countof */

#ifndef _countof
    #ifdef __cplusplus
        namespace unboost {
            template <typename T, size_t N>
            inline size_t _countof_helper(const T (&array)[N]) {
                return N;
            }
        }
        #define _countof            unboost::_countof_helper
    #else
        #define _countof(array)     (sizeof(array) / sizeof((array)[0]))
    #endif
#endif

/****************************************************************************/

#endif  /* ndef UNBOOST_CORE_H_ */
