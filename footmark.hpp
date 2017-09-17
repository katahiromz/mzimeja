// footmark.hpp --- footmark++ for C++ tracing
// This file is public domain software (PDS).
// NOTE: Footmark is not thread-safe. Don't use it in multithread function.
///////////////////////////////////////////////////////////////////////////////

#ifndef FOOTMARK_HPP_
#define FOOTMARK_HPP_   12    // Version 12

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

#ifndef NDEBUG
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
    int         m_line;
    const char *m_func;
    bool        m_flag;
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
      int       m_retval_int;
      long      m_retval_long;
      void *    m_retval_ptr;
#ifdef _WIN32
      LPARAM    m_retval_lparam;
#endif
    };

    FootmarkLocation() : m_file(NULL), m_line(0), m_func(NULL),
      m_flag(false), m_retval_type(RETVAL_NONE) { }

    FootmarkLocation(const char *file, int line, const char *func,
                     bool new_line = true) :
      m_file(file), m_line(line), m_func(func), m_flag(true),
      m_retval_type(RETVAL_NONE) { Enter(new_line); }

    FootmarkLocation(const FootmarkLocation& location) :
      m_file(location.m_file), m_line(location.m_line),
      m_func(location.m_func), m_flag(false),
      m_retval_type(location.m_retval_type),
      m_retval_ptr(location.m_retval_ptr) { }
    FootmarkLocation& operator=(const FootmarkLocation& location) {
      m_file = location.m_file;
      m_line = location.m_line;
      m_func = location.m_func;
      m_flag = false;
      m_retval_type = location.m_retval_type;
      m_retval_ptr = location.m_retval_ptr;
      return *this;
    }
    ~FootmarkLocation() { if (m_flag) { Leave(); } }
    bool emit_true() const { return true; }
  protected:
    void Enter(bool newline = true);
    void Leave();
  }; // struct FootmarkLocation

  typedef std::vector<FootmarkLocation>   FootmarkStackType;
  inline FootmarkStackType& GetFootmarkStack() {
    static FootmarkStackType s_stack;
    return s_stack;
  }

  inline void FootmarkLocation::Enter(bool newline/* = true*/) {
    GetFootmarkStack().push_back(*this);
    if (newline) {
      FootmarkDebugPrint("%s %s (%u): entering %s\n",
        std::string(GetFootmarkStack().size(), '>').c_str(),
        m_file, m_line, m_func);
    } else {
      FootmarkDebugPrint("%s %s (%u): entering %s: ",
        std::string(GetFootmarkStack().size(), '>').c_str(),
        m_file, m_line, m_func);
    }
  }
  inline void FootmarkLocation::Leave() {
    if (GetFootmarkStack().size()) {
      FootmarkDebugPrint("%s %s: leaving %s",
        std::string(GetFootmarkStack().size(), '>').c_str(),
        m_file, m_func);
      switch (m_retval_type) {
      case RETVAL_NONE:
        FootmarkDebugPrint("\n");
        break;
      case RETVAL_INT:
        FootmarkDebugPrint(" returned %d\n", m_retval_int);
        break;
      case RETVAL_LONG:
        FootmarkDebugPrint(" returned 0x%08lX\n", m_retval_long);
        break;
      case RETVAL_PTR:
        FootmarkDebugPrint(" returned %p\n", m_retval_ptr);
        break;
#ifdef _WIN32
      case RETVAL_LPARAM:
# ifdef _WIN64
        if (sizeof(LPARAM) == sizeof(LONG))
          FootmarkDebugPrint(" returned 0x%08lX\n", m_retval_lparam);
        else if (sizeof(LPARAM) == 2 * sizeof(LONG))
          FootmarkDebugPrint(" returned 0x%08lX%08lX\n",
            (LONG)(m_retval_lparam >> 32), (LONG)m_retval_lparam);
# else
        FootmarkDebugPrint(" return 0x%08lX\n", m_retval_lparam);
# endif
        break;
#endif
      }
      GetFootmarkStack().pop_back();
    } else {
      FootmarkDebugPrint("Footmark: ERROR: The stack of footmarks was broken.\n");
      FootmarkDebugPrint("NOTE: Footmark is not thread-safe.\n");
      assert(0);
    }
  }

  inline void FootmarkPrintCallStack(const char *fname, int line) {
    FootmarkDebugPrint("%s (%d): FOOTMARK_PRINT_CALL_STACK()\n", fname, line);
    FootmarkDebugPrint("### CALL STACK ###\n");
    const FootmarkStackType& stack = GetFootmarkStack();
    for (size_t i = stack.size(); i > 0; ) {
      --i;
      FootmarkDebugPrint("+ %s (%u): %s\n", stack[i].m_file, stack[i].m_line,
                         stack[i].m_func);
    }
  }
#endif  // ndef NDEBUG

///////////////////////////////////////////////////////////////////////////////
// FOOTMARK* Macros

#ifndef NDEBUG
  #if (__cplusplus >= 201103L) // C++11
    #define FOOTMARK() \
      FootmarkLocation the_footmark(__FILE__, __LINE__, __func__);
  #else // until C++11
    #define FOOTMARK() \
      FootmarkLocation the_footmark(__FILE__, __LINE__, __FUNCTION__);
  #endif
  #define FOOTMARK_POINT() FootmarkDebugPrint("%s (%d): FOOTMARK_POINT()\n", \
                                              __FILE__, __LINE__)
  #define FOOTMARK_PRINT_CALL_STACK() FootmarkPrintCallStack(__FILE__, __LINE__)
  #if (__cplusplus >= 201103L) // C++11
    #define FOOTMARK_FORMAT \
      FootmarkLocation the_footmark(__FILE__, __LINE__, __func__, false); \
      FootmarkDebugPrint
  #else
    #define FOOTMARK_FORMAT \
      FootmarkLocation the_footmark(__FILE__, __LINE__, __FUNCTION__, false); \
      FootmarkDebugPrint
  #endif
  #define FOOTMARK_RETURN_INT(retval) \
    do { \
      assert(the_footmark.emit_true()); \
      the_footmark.m_retval_type = FootmarkLocation::RETVAL_INT; \
      the_footmark.m_retval_int = (int)(retval); \
      return the_footmark.m_retval_int; \
    } while (0)
  #define FOOTMARK_RETURN_LONG(retval) \
    do { \
      assert(the_footmark.emit_true()); \
      the_footmark.m_retval_type = FootmarkLocation::RETVAL_LONG; \
      the_footmark.m_retval_long = (long)(retval); \
      return the_footmark.m_retval_long; \
    } while (0)
  #define FOOTMARK_RETURN_PTR(ptrtype,retval) \
    do { \
      assert(the_footmark.emit_true()); \
      the_footmark.m_retval_type = FootmarkLocation::RETVAL_PTR; \
      the_footmark.m_retval_ptr = (void *)(retval); \
      return (ptrtype)the_footmark.m_retval_ptr; \
    } while (0)
  #ifdef _WIN32
    #define FOOTMARK_RETURN_LPARAM(retval) \
      do { \
        assert(the_footmark.emit_true()); \
        the_footmark.m_retval_type = FootmarkLocation::RETVAL_LPARAM; \
        the_footmark.m_retval_lparam = (LPARAM)(retval); \
        return the_footmark.m_retval_lparam; \
      } while (0)
  #endif
#else   // def NDEBUG
  #define FOOTMARK()                          /*empty*/
  #define FOOTMARK_POINT()                    /*empty*/
  #define FOOTMARK_PRINT_CALL_STACK()         /*empty*/
  #define FOOTMARK_FORMAT                     /*empty*/
  #define FOOTMARK_RETURN_INT(retval)         return retval
  #define FOOTMARK_RETURN_LONG(retval)        return retval
  #define FOOTMARK_RETURN_PTR(ptrtype,retval) return retval
  #ifdef _WIN32
    #define FOOTMARK_RETURN_LPARAM(retval)    return retval
  #endif
#endif  // def NDEBUG

///////////////////////////////////////////////////////////////////////////////

#endif  // ndef FOOTMARK_HPP_
