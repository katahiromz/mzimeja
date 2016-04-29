// WStringRef.hpp --- constant wide string reference
// This file is public domain software (PDS).
//////////////////////////////////////////////////////////////////////////////

#ifndef WSTRINGREF_HPP_
#define WSTRINGREF_HPP_

#include <cstring>  // for wcslen
#include <cassert>  // for assert

//////////////////////////////////////////////////////////////////////////////

class WStringRef {
public:
    typedef wchar_t         value_type;
    typedef size_t          size_type;
    typedef const wchar_t&  const_reference;
    typedef const wchar_t * const_pointer;
    typedef const_pointer   const_iterator;

public:
    WStringRef() : m_ptr(NULL), m_len(0) {}
    WStringRef(const wchar_t *ptr) : m_ptr(ptr) {
        using namespace std;
        m_len = wcslen(ptr);
    }
    WStringRef(const WStringRef& str) : m_ptr(str.m_ptr), m_len(str.m_len) {}

    bool empty() const { return size() == 0; }
    size_t size() const { return m_len; }

    WStringRef& operator=(const wchar_t *ptr) {
        using namespace std;
        m_ptr = ptr;
        m_len = wcslen(ptr);
        return *this;
    }

    const wchar_t& operator[](size_t index) const {
        assert(index <= m_len);
        return m_ptr[index];
    }
    const wchar_t *c_str() const { return m_ptr; }

    const wchar_t *begin() const { return &m_ptr[0]; }
    const wchar_t *end()   const { return &m_ptr[m_len]; }

protected:
    const wchar_t *     m_ptr;
    size_t              m_len;
}; // class WStringRef

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef WSTRINGREF_HPP_

//////////////////////////////////////////////////////////////////////////////
