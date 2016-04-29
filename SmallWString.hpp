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
    typedef wchar_t         value_type;
    typedef size_t          size_type;
    typedef wchar_t&        reference;
    typedef const wchar_t&  const_reference;
    typedef wchar_t *       pointer;
    typedef const wchar_t * const_pointer;
    typedef pointer         iterator;
    typedef const_pointer   const_iterator;

public:
    SmallWString() { m_buf[0] = 0; }
    SmallWString(const wchar_t *str) {
        assign(str);
    }
    SmallWString(const wchar_t *str, size_t len) {
        assign(str, len);
    }
    SmallWString(const std::wstring& str) {
        assign(str.c_str(), str.size());
    }
    SmallWString(const SmallWString& str) {
        assign(str);
    }

    bool empty() const { return size() == 0; }
    size_t size() const {
        for (size_t index = 0; index < c_capacity; ++index) {
            if (m_buf[index] == 0) return index;
        }
        return c_capacity;
    }
    void clear() {
        m_buf[0] = 0;
    }

    SmallWString& operator=(const wchar_t *str) {
        return assign(str);
    }
    SmallWString& assign(const wchar_t *str) {
        wchar_t *pch = m_buf;
        wchar_t *pchEnd = pch + c_capacity;
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
    }
    SmallWString& assign(size_t count, wchar_t ch) {
        if (count > c_capacity) {
            count = c_capacity;
        }
        if (count < c_capacity) {
            m_buf[count] = 0;
        }
        while (count) {
            m_buf[--count] = ch;
        }
        return *this;
    }
    SmallWString& assign(const wchar_t *str, size_t len) {
        using namespace std;
        if (len > c_capacity) {
            len = c_capacity;
        }
        memcpy(m_buf, str, len * sizeof(wchar_t));
        if (len < c_capacity) {
            m_buf[len] = 0;
        }
        return *this;
    }

    SmallWString& erase() {
        clear();
        return *this;
    }
    SmallWString& erase(size_t index) {
        assert(index <= c_capacity);
        m_buf[index] = 0;
        return *this;
    }
    SmallWString& erase(size_t index, size_t count) {
        assert(index <= c_capacity);
        assert(index + count <= c_capacity);
        size_t k;
        for (k = index; k < c_capacity - count; ++k) {
            m_buf[k] = m_buf[k + count];
        }
        if (k < c_capacity) m_buf[k] = 0;
        return *this;
    }

    SmallWString& insert(size_t index, size_t count, wchar_t ch) {
        using namespace std;
        if (index < c_capacity) {
            if (index + count > c_capacity) {
                count = c_capacity - index;
            }
            const size_t span = c_capacity - (index + count);
            memmove(&m_buf[index + count], &m_buf[index], span);
            for (size_t k = index; k < index + count; ++k) {
                m_buf[k] = ch;
            }
        }
    }
    SmallWString& insert(const wchar_t *pos, wchar_t ch) {
        using namespace std;
        assert(m_buf <= pos);
        const size_t index = pos - m_buf;
        if (index < c_capacity) {
            const size_t span = c_capacity - (index + 1);
            memmove(&m_buf[index + 1], &m_buf[index], span);
            m_buf[index] = ch;
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
    const wchar_t *c_str() const { return m_buf; }
    wchar_t *data() { return m_buf; }

    wchar_t *begin()    { return &m_buf[0]; }
    wchar_t *end()      { return &m_buf[size()]; }
    const wchar_t *begin() const { return &m_buf[0]; }
    const wchar_t *end()   const { return &m_buf[size()]; }

    std::wstring str() const { return std::wstring(m_buf, size()); }

protected:
    static const size_t c_capacity = 6;
    wchar_t m_buf[c_capacity];
}; // class SmallWString

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef SMALLWSTRING_HPP_

//////////////////////////////////////////////////////////////////////////////
