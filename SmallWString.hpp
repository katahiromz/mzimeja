#ifndef SMALLWSTRING_HPP_
#define SMALLWSTRING_HPP_

#include <cstring>  // for memcpy, wcslen
#include <cassert>  // for assert

class SmallWString {
public:
    SmallWString() { m_buf[0] = 0; }
    SmallWString(const wchar_t *str) {
        assign(str);
    }
    SmallWString(const wchar_t *str, size_t len) {
        assign(str, len);
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

    SmallWString& operator=(const wchar_t *str) {
        return assign(str);
    }
    SmallWString& assign(const wchar_t *str) {
        return assign(str, wcslen(str));
    }
    SmallWString& assign(const SmallWString& str) {
        using namespace std;
        memcpy(m_buf, str.m_buf, c_capacity * sizeof(wchar_t));
    }
    SmallWString& assign(size_t count, wchar_t ch) {
        if (count > c_capacity) {
            count = c_capacity;
        }
        for (size_t i = 0; i < count; ++i) {
            m_buf[i] = ch;
        }
        if (count < c_capacity) {
            m_buf[count] = 0;
        }
        return *this;
    }
    SmallWString& assign(const wchar_t *str, size_t len) {
        if (len > c_capacity) {
            len = c_capacity;
        }
        for (size_t i = 0; i < len; ++i) {
            m_buf[i] = str[i];
        }
        if (len < c_capacity) {
            m_buf[len] = 0;
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

protected:
    static const size_t c_capacity = 6;
    wchar_t m_buf[c_capacity];
}; // class SmallWString

#endif  // ndef SMALLWSTRING_HPP_
