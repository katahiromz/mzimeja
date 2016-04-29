#ifndef WSTRINGREF_HPP_
#define WSTRINGREF_HPP_

#include <cstring>  // for wcslen

class WStringRef {
public:
    WStringRef() : m_ptr(NULL), m_len(0) {}
    WStringRef(const wchar_t *ptr) : m_ptr(ptr) {
        using namespace std;
        m_len = wcslen(ptr);
    }
    WStringRef(const wchar_t *ptr, size_t len) : m_ptr(ptr), m_len(len) {}
    WStringRef(const WStringRef& str) : m_ptr(str.m_ptr), m_len(str.m_len) {}

    bool empty() const { return size() == 0; }
    size_t size() const { return m_len; }

    SmallWString& operator=(const wchar_t *ptr) {
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

protected:
    const wchar_t *m_ptr;
    size_t m_len;
}; // class WStringRef

#endif	// ndef WSTRINGREF_HPP_
