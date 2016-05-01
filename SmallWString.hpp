// SmallWString.hpp --- small wide string
// This file is public domain software (PDS).
//////////////////////////////////////////////////////////////////////////////

#ifndef SMALLWSTRING_HPP_
#define SMALLWSTRING_HPP_

#include <cstring>  // for memcpy
#include <cassert>  // for assert

#include <string>   // for std::string

//////////////////////////////////////////////////////////////////////////////

class SmallWString {
public:
    static const size_t c_capacity = 6;

    typedef wchar_t         value_type;
    typedef size_t          size_type;
    typedef wchar_t&        reference;
    typedef const wchar_t&  const_reference;
    typedef wchar_t *       pointer;
    typedef const wchar_t * const_pointer;
    typedef pointer         iterator;
    typedef const_pointer   const_iterator;

public:
    SmallWString()                                  { m_buf[0] = 0; }
    SmallWString(size_t count, wchar_t ch)          { assign(count, ch); }
    SmallWString(const wchar_t *str)                { assign(str); }
    SmallWString(const wchar_t *str, size_t len)    { assign(str, len); }
    SmallWString(const std::wstring& str) { assign(str.c_str(), str.size()); }
    SmallWString(const SmallWString& str) { assign(str); }

    bool empty() const { return size() == 0; }
    size_t size() const {
        size_t index;
        for (index = 0; index < c_capacity; ++index) {
            if (m_buf[index] == 0) break;
        }
        return index;
    }
    void clear() { m_buf[0] = 0; }
    void resize(size_t count) { erase(count); }

    SmallWString& operator=(const wchar_t *str) { return assign(str); }
    SmallWString& assign(const wchar_t *str) {
        wchar_t *pch = m_buf, *pchEnd = pch + c_capacity;
        while (*str) {
            *pch++ = *str++;
            if (pch == pchEnd) return *this;
        }
        *pch = 0;
        return *this;
    }
    SmallWString& assign(const SmallWString& str) {
        using namespace std;
        memcpy(m_buf, str.m_buf, c_capacity * sizeof(wchar_t));
        return *this;
    }
    SmallWString& assign(size_t count, wchar_t ch) {
        if (count > c_capacity) count = c_capacity;
        if (count < c_capacity) m_buf[count] = 0;
        while (count) {
            m_buf[--count] = ch;
        }
        return *this;
    }
    SmallWString& assign(const wchar_t *str, size_t len) {
        using namespace std;
        if (len > c_capacity) len = c_capacity;
        memcpy(m_buf, str, len * sizeof(wchar_t));
        if (len < c_capacity) m_buf[len] = 0;
        return *this;
    }

    SmallWString& erase() { clear(); return *this; }
    SmallWString& erase(size_t index) {
        if (index < c_capacity) m_buf[index] = 0;
        return *this;
    }
    SmallWString& erase(size_t index, size_t count) {
        for (size_t k = index; k + count < c_capacity; ++k) {
            m_buf[k] = m_buf[k + count];
        }
        if (index + count < c_capacity) m_buf[index + count] = 0;
        return *this;
    }

    SmallWString& operator+=(wchar_t ch) {
        size_t len = size();
        if (len < c_capacity) {
            m_buf[len++] = ch;
            if (len < c_capacity) {
                m_buf[len] = 0;
            }
        }
        return *this;
    }
    SmallWString& operator+=(const wchar_t *str) {
        return insert(size(), str);
    }

    SmallWString& insert(size_t index, size_t count, wchar_t ch) {
        using namespace std;
        if (index < c_capacity) {
            if (index + count > c_capacity) {
                count = c_capacity - index;
            }
            const size_t span = c_capacity - (index + count);
            const size_t siz = span * sizeof(wchar_t);
            memmove(&m_buf[index + count], &m_buf[index], siz);
            for (size_t k = index; k < index + count; ++k) {
                m_buf[k] = ch;
            }
        }
        return *this;
    }
    SmallWString& insert(const wchar_t *pos, wchar_t ch) {
        using namespace std;
        assert(m_buf <= pos);
        const size_t index = pos - m_buf;
        if (index < c_capacity) {
            const size_t span = c_capacity - (index + 1);
            const size_t siz = span * sizeof(wchar_t);
            memmove(&m_buf[index + 1], &m_buf[index], siz);
            m_buf[index] = ch;
        }
        return *this;
    }
    SmallWString& insert(size_t index, const wchar_t *str) {
        using namespace std;
        if (index < c_capacity) {
            size_t count = wcslen(str);
            if (index + count > c_capacity) {
                count = c_capacity - index;
            }
            const size_t span = c_capacity - (index + count);
            const size_t siz = span * sizeof(wchar_t);
            memmove(&m_buf[index + count], &m_buf[index], siz);
            for (size_t k = index; k < index + count; ++k) {
                m_buf[k] = str[k - index];
            }
        }
        return *this;
    }

    wchar_t& operator[](size_t index) {
        assert(index < c_capacity);
        return m_buf[index];
    }
    const wchar_t& operator[](size_t index) const {
        assert(index < c_capacity);
        return m_buf[index];
    }
          wchar_t *data()        { return m_buf; }
    const wchar_t *c_str() const { return m_buf; }

    wchar_t *begin()    { return &m_buf[0]; }
    wchar_t *end()      { return &m_buf[size()]; }
    const wchar_t *begin() const { return &m_buf[0]; }
    const wchar_t *end()   const { return &m_buf[size()]; }

    std::wstring str() const { return std::wstring(m_buf, size()); }

protected:
    wchar_t m_buf[c_capacity];
}; // class SmallWString

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef SMALLWSTRING_HPP_

//////////////////////////////////////////////////////////////////////////////