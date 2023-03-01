#pragma once

template <typename T_CHAR>
void str_trim_right(std::basic_string<T_CHAR>& str, const T_CHAR* spaces)
{
    typedef std::basic_string<T_CHAR> string_type;
    size_t j = str.find_last_not_of(spaces);
    if (j == string_type::npos)
    {
        str.clear();
    }
    else
    {
        str = str.substr(0, j + 1);
    }
}

template <typename T_STR_CONTAINER>
void
str_split(T_STR_CONTAINER& container,
    const typename T_STR_CONTAINER::value_type& str,
    const typename T_STR_CONTAINER::value_type& chars)
{
    container.clear();
    size_t i = 0, k = str.find_first_of(chars);
    while (k != T_STR_CONTAINER::value_type::npos)
    {
        container.push_back(str.substr(i, k - i));
        i = k + 1;
        k = str.find_first_of(chars, i);
    }
    container.push_back(str.substr(i));
}

template <typename T_STR>
bool
str_replace_all(T_STR& str, const T_STR& from, const T_STR& to)
{
    bool ret = false;
    size_t i = 0;
    for (;;) {
        i = str.find(from, i);
        if (i == T_STR::npos)
            break;
        ret = true;
        str.replace(i, from.size(), to);
        i += to.size();
    }
    return ret;
}

template <typename T_STR>
inline bool
str_replace_all(T_STR& str, const typename T_STR::value_type* from, const typename T_STR::value_type* to)
{
    return str_replace_all(str, T_STR(from), T_STR(to));
}
