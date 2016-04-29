#ifndef SMALLWSTRING_HPP_
#define SMALLWSTRING_HPP_

#include <cstring>  // for memcpy, wcslen

class SmallWString {
public:
    SmallWString() { m_buf[0] = 0; }
    SmallWString(const wchar_t *str) {
        *this = str;
    }
    SmallWString(const wchar_t *str, size_t len) {
        assign(str, len);
    }
    SmallWString(const SmallWString& str) {
        using namespace std;
        memcpy(m_buf, str.m_buf, c_capacity * sizeof(wchar_t));
    }

    bool empty() const { return size() == 0; }
    size_t size() const {
        for (size_t i = 0; i < capacity; ++i) {
            if (m_buf[i] == 0) return i;
        }
        return capacity;
    }

    SmallWString& operator=(const wchar_t *str) {
        using namespace std;
        size_t len = wcslen(str);
        if (len > capacity) len = capacity;
        memcpy(m_buf, str, len * sizeof(wchar_t));
        if (len < capacity) {
            m_buf[len] = 0;
        }
        return *this;
    }
    SmallWString& assign(size_t count, wchar_t ch) {
        if (count > capacity) {
            count = capacity;
        }
        for (size_t i = 0; i < count; ++i) {
            m_buf[i] = ch;
        }
        if (count < capacity) {
            m_buf[count] = 0;
        }
        return *this;
    }
    SmallWString& assign(const wchar_t *str, size_t len) {
        if (len > capacity) {
            len = capacity;
        }
        for (size_t i = 0; i < len; ++i) {
            m_buf[i] = str[i];
        }
        if (len < capacity) {
            m_buf[len] = 0;
        }
        return *this;
    }

    wchar_t& operator[](size_t index) {
        assert(index < capacity);
        return m_buf[index];
    }
    const wchar_t& operator[](size_t index) const {
        assert(index < capacity);
        return m_buf[index];
    }
    const wchar_t *c_str() const { return m_buf; }
    wchar_t *data() { return m_buf; }

protected:
    static const size_t c_capacity = 6;
    wchar_t m_buf[capacity];
}; // class SmallWString

#endif  // ndef SMALLWSTRING_HPP_
