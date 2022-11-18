// footmark.hpp --- footmark++ for C++ tracing
// This file is public domain software (PDS).
// NOTE: Footmark is not thread-safe. Don't use it in multithread function.
///////////////////////////////////////////////////////////////////////////////

#ifndef FOOTMARK_HPP_
#define FOOTMARK_HPP_   13    // Version 13

#ifndef __cplusplus
  #error This library (footmark++) needs C++. You lose.
#endif

///////////////////////////////////////////////////////////////////////////////
// FootmarkDebugPrint

#ifndef FootmarkDebugPrint
  #ifdef DebugPrintA
    #define FootmarkDebugPrint  DebugPrintA
  #elif defined(DebugPrint)
    #define FootmarkDebugPrint  DebugPrint
  #else
    #include <cstdio>
    #define FootmarkDebugPrint  printf
  #endif
#endif

///////////////////////////////////////////////////////////////////////////////
// FootmarkLocation and FootmarkPrintCallStack (on debugging)

#ifdef MZIMEJA_DEBUG_OUTPUT
  #include <vector>   // for std::vector
  #include <string>   // for std::string and std::wstring
  #include <cassert>  // for assert
  #ifdef _WIN32
    #ifndef _INC_WINDOWS
      #include <windows.h>
    #endif
  #endif

struct FootmarkLocation {
    const char *m_file;
    int m_line;
    const char *m_func;
    bool m_entered;
    enum {
        RETVAL_NONE,
        RETVAL_INT,
        RETVAL_LONG,
        RETVAL_PTR
#ifdef _WIN32
        , RETVAL_LPARAM
#endif
    }           m_retval_type;
    union {
        int m_retval_int;
        long m_retval_long;
        void *    m_retval_ptr;
#ifdef _WIN32
        LPARAM m_retval_lparam;
#endif
    };

    FootmarkLocation() : m_file(NULL), m_line(0), m_func(NULL),
        m_entered(false), m_retval_type(RETVAL_NONE) {
    }

    FootmarkLocation(const char *file, int line, const char *func) :
        m_file(file), m_line(line), m_func(func), m_entered(true),
        m_retval_type(RETVAL_NONE) {
        Enter();
    }

    FootmarkLocation(const FootmarkLocation& location) :
        m_file(location.m_file), m_line(location.m_line),
        m_func(location.m_func), m_entered(false),
        m_retval_type(location.m_retval_type),
        m_retval_ptr(location.m_retval_ptr) {
    }
    FootmarkLocation& operator=(const FootmarkLocation& location) {
        m_file = location.m_file;
        m_line = location.m_line;
        m_func = location.m_func;
        m_entered = false;
        m_retval_type = location.m_retval_type;
        m_retval_ptr = location.m_retval_ptr;
        return *this;
    }
    ~FootmarkLocation() {
        if (m_entered) { Leave(); }
    }
protected:
    void Enter();
    void Leave();
};   // struct FootmarkLocation

inline void FootmarkLocation::Enter() {
    FootmarkDebugPrint("%s (%u): %s: Enter\n", m_file, m_line, m_func);
}
inline void FootmarkLocation::Leave() {
    switch (m_retval_type) {
    case RETVAL_NONE:
        FootmarkDebugPrint("%s: %s: Leave\n", m_file, m_func);
        break;
    case RETVAL_INT:
        FootmarkDebugPrint("%s: %s: Leave: %d\n", m_file, m_func, m_retval_int);
        break;
    case RETVAL_LONG:
        FootmarkDebugPrint("%s: %s: Leave: 0x%08lX\n", m_file, m_func, m_retval_long);
        break;
    case RETVAL_PTR:
        FootmarkDebugPrint("%s: %s: Leave: %p\n", m_file, m_func, m_retval_ptr);
        break;
#ifdef _WIN32
    case RETVAL_LPARAM:
        FootmarkDebugPrint("%s: %s: Leave: %p\n", m_file, m_func, m_retval_lparam);
        break;
#endif
    }
}
#endif  // def MZIMEJA_DEBUG_OUTPUT

///////////////////////////////////////////////////////////////////////////////
// FOOTMARK* Macros

#if defined(MZIMEJA_DEBUG_OUTPUT)
  #if (__cplusplus >= 201103L) // C++11
    #define FOOTMARK() \
        FootmarkLocation the_footmark(__FILE__, __LINE__, __func__);
  #else // until C++11
    #define FOOTMARK() \
        FootmarkLocation the_footmark(__FILE__, __LINE__, __FUNCTION__);
  #endif
  #define FOOTMARK_POINT() FootmarkDebugPrint("%s (%d): FOOTMARK_POINT()\n", \
                                              __FILE__, __LINE__)
  #if (__cplusplus >= 201103L) // C++11
    #define FOOTMARK_FORMAT \
        FootmarkLocation the_footmark(__FILE__, __LINE__, __func__); \
        FootmarkDebugPrint
  #else
    #define FOOTMARK_FORMAT \
        FootmarkLocation the_footmark(__FILE__, __LINE__, __FUNCTION__); \
        FootmarkDebugPrint
  #endif
  #define FOOTMARK_RETURN_INT(retval) do { \
        the_footmark.m_retval_type = FootmarkLocation::RETVAL_INT; \
        the_footmark.m_retval_int = (int)(retval); \
        return the_footmark.m_retval_int; \
} while (0)
  #define FOOTMARK_RETURN_LONG(retval) do { \
        the_footmark.m_retval_type = FootmarkLocation::RETVAL_LONG; \
        the_footmark.m_retval_long = (long)(retval); \
        return the_footmark.m_retval_long; \
} while (0)
  #define FOOTMARK_RETURN_PTR(ptrtype,retval) do { \
        the_footmark.m_retval_type = FootmarkLocation::RETVAL_PTR; \
        the_footmark.m_retval_ptr = (void *)(retval); \
        return (ptrtype)the_footmark.m_retval_ptr; \
} while (0)
  #ifdef _WIN32
    #define FOOTMARK_RETURN_LPARAM(retval) do { \
        the_footmark.m_retval_type = FootmarkLocation::RETVAL_LPARAM; \
        the_footmark.m_retval_lparam = (LPARAM)(retval); \
        return the_footmark.m_retval_lparam; \
} while (0)
  #endif
#else   // !def MZIMEJA_DEBUG_OUTPUT
  #define FOOTMARK()                          /*empty*/
  #define FOOTMARK_POINT()                    /*empty*/
  #define FOOTMARK_FORMAT                     /*empty*/
  #define FOOTMARK_RETURN_INT(retval)         return retval
  #define FOOTMARK_RETURN_LONG(retval)        return retval
  #define FOOTMARK_RETURN_PTR(ptrtype,retval) return retval
  #ifdef _WIN32
    #define FOOTMARK_RETURN_LPARAM(retval)    return retval
  #endif
#endif  // !def MZIMEJA_DEBUG_OUTPUT

///////////////////////////////////////////////////////////////////////////////

#endif  // ndef FOOTMARK_HPP_
