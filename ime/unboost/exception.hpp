// exception.hpp --- Unboost exceptions
//////////////////////////////////////////////////////////////////////////////

#ifndef UNBOOST_EXCEPTION_HPP_
#define UNBOOST_EXCEPTION_HPP_

#include "unboost.h"
#include <exception>
#include <stdexcept>
#include <cstdio>

#ifdef UNBOOST_USE_CXX11
    #include <memory>
#elif defined(UNBOOST_USE_BOOST)
    #include <boost/lexical_cast.hpp>           // for boost::bad_lexical_cast
    #include <boost/smart_ptr/bad_weak_ptr.hpp> // for boost::bad_weak_ptr
#endif

namespace unboost {
    using std::exception;
    using std::invalid_argument;
    using std::out_of_range;
    using std::runtime_error;
    using std::range_error;

    #ifdef UNBOOST_USE_BOOST
        using boost::bad_lexical_cast;
    #else
        class bad_lexical_cast : public exception {
        public:
            bad_lexical_cast() { }
            virtual const char *what() const UNBOOST_NOEXCEPT{
                return "lexical_cast";
            }
        };
    #endif

    #ifdef UNBOOST_USE_CXX11
        using std::bad_weak_ptr;
    #elif defined(UNBOOST_USE_BOOST)
        using boost::bad_weak_ptr;
    #else
        class bad_weak_ptr : public exception {
        public:
            bad_weak_ptr() { }
            virtual ~bad_weak_ptr() { }
            virtual const char *what() const UNBOOST_NOEXCEPT {
                return "unboost::bad_weak_ptr";
            }
        };
    #endif
} // namespace unboost

#endif  // ndef UNBOOST_EXCEPTION_HPP_
