// footmark.hpp --- footmark for C++ debugging
// This file is public domain software (PDS).
// NOTE: Footmark is not thread-safe. Don't use it in multithread function.
///////////////////////////////////////////////////////////////////////////////

#ifndef FOOTMARK_HPP_
#define FOOTMARK_HPP_   2   // Version 2

#ifndef __cplusplus
  #error This library (footmark) needs C++. You lose.
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
  #include <cassert>  // for assert

  struct FootmarkLocation {
    const char *m_file;
    int         m_line;
    const char *m_func;
    bool        m_flag;
    FootmarkLocation() :
      m_file(NULL), m_line(0), m_func(NULL), m_flag(false) {}
    FootmarkLocation(const char *file, int line, const char *func) :
      m_file(file), m_line(line), m_func(func), m_flag(true) { Enter(); }
    FootmarkLocation(const FootmarkLocation& location) :
      m_file(location.m_file), m_line(location.m_line),
      m_func(location.m_func), m_flag(false) {}
    FootmarkLocation& operator=(const FootmarkLocation& location) {
      m_file = location.m_file;
      m_line = location.m_line;
      m_func = location.m_func;
      m_flag = false;
      return *this;
    }
    ~FootmarkLocation() { if (m_flag) { Leave(); } }
    void Enter();
    void Leave();
  }; // struct FootmarkLocation

  typedef std::vector<FootmarkLocation>   FootmarkStackType;
  inline FootmarkStackType& GetFootmarkStack() {
    static FootmarkStackType s_stack;
    return s_stack;
  }

  inline void FootmarkLocation::Enter() {
    GetFootmarkStack().push_back(*this);
    FootmarkDebugPrint("%s (%u): entering %s\n", m_file, m_line, m_func);
  }
  inline void FootmarkLocation::Leave() {
    if (GetFootmarkStack().size()) {
      FootmarkDebugPrint("%s: leaving %s\n", m_file, m_func);
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
    for (size_t i = 0; i < stack.size(); ++i) {
      FootmarkDebugPrint("+ %s (%u): %s\n", stack[i].m_file, stack[i].m_line,
                         stack[i].m_func);
    }
  }
#endif  // ndef NDEBUG

///////////////////////////////////////////////////////////////////////////////
// FOOTMARK(), FOOTMARK_PRINT_CALL_STACK() macros

#ifndef NDEBUG
  #if (__cplusplus >= 201103L) // C++11
    #define FOOTMARK() \
      FootmarkLocation \
        object_for_debugging_##__LINE__(__FILE__, __LINE__, __func__);
  #else // until C++11
    #define FOOTMARK() \
      FootmarkLocation \
        object_for_debugging_##__LINE__(__FILE__, __LINE__, __FUNCTION__);
  #endif
  #define FOOTMARK_PRINT_CALL_STACK() FootmarkPrintCallStack(__FILE__, __LINE__)
#else   // def NDEBUG
  #define FOOTMARK()                  /*empty*/
  #define FOOTMARK_PRINT_CALL_STACK() /*empty*/
#endif  // def NDEBUG

///////////////////////////////////////////////////////////////////////////////

#endif  // ndef FOOTMARK_HPP_

///////////////////////////////////////////////////////////////////////////////
