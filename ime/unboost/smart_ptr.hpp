// smart_ptr.hpp --- Unboost smart pointers
//////////////////////////////////////////////////////////////////////////////

#ifndef UNBOOST_SMART_PTR_HPP_
#define UNBOOST_SMART_PTR_HPP_

#if (_MSC_VER > 1000)
    #pragma once
#endif

#include "unboost.h"
#include "exception.hpp"    // for unboost::bad_weak_ptr

// If not choosed, choose one
#if ((defined(UNBOOST_USE_CXX11_SMART_PTR) + defined(UNBOOST_USE_TR1_SMART_PTR) + defined(UNBOOST_USE_BOOST_SMART_PTR) + defined(UNBOOST_USE_UNBOOST_SMART_PTR)) == 0)
    #ifdef UNBOOST_USE_CXX11
        #define UNBOOST_USE_CXX11_SMART_PTR
    #elif defined(UNBOOST_USE_TR1)
        #define UNBOOST_USE_TR1_SMART_PTR
    #elif defined(UNBOOST_USE_BOOST)
        #define UNBOOST_USE_BOOST_SMART_PTR
    #else
        #ifdef UNBOOST_CXX11    // C++11
            #define UNBOOST_USE_CXX11_SMART_PTR
        #elif defined(_MSC_VER)
            #if (1500 <= _MSC_VER) && (_MSC_VER <= 1600)
                // Visual C++ 2008 SP1 and 2010
                #ifndef UNBOOST_NO_TR1
                    #define UNBOOST_USE_TR1_SMART_PTR
                #else
                    #define UNBOOST_USE_UNBOOST_SMART_PTR
                #endif
                // NOTE: On MSVC 2008, you needs SP1.
            #elif (_MSC_VER >= 1700)
                // Visual C++ 2012 and later
                #ifndef UNBOOST_NO_CXX11
                    #define UNBOOST_USE_CXX11_SMART_PTR
                #else
                    #define UNBOOST_USE_UNBOOST_SMART_PTR
                #endif
            #else
                #define UNBOOST_USE_UNBOOST_SMART_PTR
            #endif
        #elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
            // GCC 4.3 and later
            #ifndef UNBOOST_NO_TR1
                #define UNBOOST_USE_TR1_SMART_PTR
            #else
                #define UNBOOST_USE_UNBOOST_SMART_PTR
            #endif
        #else
            #define UNBOOST_USE_UNBOOST_SMART_PTR
        #endif
    #endif  // ndef UNBOOST_USE_CXX11
#endif

// Adapt choosed one
#ifdef UNBOOST_USE_CXX11_SMART_PTR
    #include <memory>       // for std::shared_ptr, ...
    namespace unboost {
        using std::shared_ptr;
        using std::make_shared;
        using std::static_pointer_cast;
        using std::dynamic_pointer_cast;
        using std::unique_ptr;
        using std::weak_ptr;
        using std::get_deleter;
    } // namespace unboost
#elif defined(UNBOOST_USE_TR1_SMART_PTR)
    #ifdef _MSC_VER
        #include <memory>       // for std::tr1::shared_ptr, ...
    #else
        #include <tr1/memory>   // for std::tr1::shared_ptr, ...
    #endif
    namespace unboost {
        using std::tr1::shared_ptr;
        #define UNBOOST_NEED_UNBOOST_MAKE_SHARED
        using std::tr1::static_pointer_cast;
        using std::tr1::dynamic_pointer_cast;
        using std::tr1::weak_ptr;
        using std::tr1::get_deleter;

        template <typename T>
        struct default_delete {
            typedef default_delete<T> self_type;
            void operator()(T *ptr) {
                if (sizeof(T) > 0) {
                    delete ptr;
                }
            }
            template <typename T2>
            void operator()(T2 *ptr) { }
        };

        template <typename T>
        struct default_delete<T[]> {
            typedef default_delete<T[]> self_type;
            void operator()(T *ptr) {
                if (sizeof(T) > 0) {
                    delete[] ptr;
                }
            }
        private:
            template <typename T2>
            void operator()(T2 *ptr) { }
        };

        template <typename T, typename DELETER = default_delete<T> >
        class unique_ptr;

        #define UNBOOST_NEED_UNBOOST_UNIQUE_PTR
    } // namespace unboost
#elif defined(UNBOOST_USE_BOOST_SMART_PTR)
    #include <boost/smart_ptr.hpp>
    #include <boost/smart_ptr/make_shared.hpp>
    #include <boost/move/unique_ptr.hpp>
    #include <boost/pointer_cast.hpp>
    #ifdef UNBOOST_FIX_UNIQUE_PTR
        namespace boost {
            namespace interprocess {
                template <typename T>
                struct default_delete : checked_deleter<T> { };
                template <typename T>
                struct default_delete<T[]> : checked_array_deleter<T> { };
                template <typename T, typename D = default_delete<T> >
                class unique_ptr;
            } // namespace interprocess
        } // namespace boost
    #endif  // def UNBOOST_FIX_UNIQUE_PTR
    namespace unboost {
        using boost::shared_ptr;
        using boost::make_shared;
        using boost::static_pointer_cast;
        using boost::dynamic_pointer_cast;
        using boost::movelib::unique_ptr;
        using boost::weak_ptr;
        using boost::get_deleter;
    } // namespace unboost
#elif defined(UNBOOST_USE_UNBOOST_SMART_PTR)
    #include <memory>       // for std::shared_ptr, ...
    #include "swap.hpp"     // for unboost::swap
    namespace unboost {
        struct _static_tag { };
        struct _const_tag { };
        struct _dynamic_tag { };

        #if (__cplusplus >= 201103L)    // C++11
            using std::default_delete;
        #else   // not C++11
            template <typename T>
            struct default_delete {
                typedef default_delete<T> self_type;
                void operator()(T *ptr) {
                    if (sizeof(T) > 0) {
                        delete ptr;
                    }
                }
                template <typename T2>
                void operator()(T2 *ptr) { }
            };

            template <typename T>
            struct default_delete<T[]> {
                typedef default_delete<T[]> self_type;
                void operator()(T *ptr) {
                    if (sizeof(T) > 0) {
                        delete[] ptr;
                    }
                }
            private:
                template <typename T2>
                void operator()(T2 *ptr) { }
            };
        #endif  // not C++11

        class _ref_count_base {
        public:
            _ref_count_base() : _m_uses(1), _m_weaks(1) { }
            virtual ~_ref_count_base() { }

            bool _inc_ref_nz() {
                long count = _m_uses;
                if (count == 0)
                    return false;
                _m_uses = count + 1;
                return true;
            }

            void _inc_ref() {
                _m_uses++;
            }
            void _inc_wref() {
                _m_weaks++;
            }

            void _dec_ref() {
                if (--_m_uses == 0)
                {
                    _destroy();
                    _dec_wref();
                }
            }
            void _dec_wref() {
                if (--_m_weaks == 0)
                    _delete_this();
            }

            long _use_count() const     { return _m_uses; }
            bool _expired() const       { return _m_uses == 0; }
        protected:
            long _m_uses;
            long _m_weaks;
            virtual void _destroy() = 0;
            virtual void _delete_this() = 0;
            virtual void *_get_deleter() { return NULL; }
        }; // _ref_count_base

        template <typename T>
        class _ref_count : public _ref_count_base {
        public:
            _ref_count(T *ptr) : _m_ptr(ptr) { }
        protected:
            T *_m_ptr;
            virtual void _destroy() {
                delete _m_ptr;
            }
            virtual void _delete_this() {
                delete this;
            }
        }; // _ref_count<T>

        template <typename T, typename DELETER>
        class _ref_count_del : public _ref_count_base {
        public:
            typedef _ref_count_del<T, DELETER> self_type;
            _ref_count_del(T *ptr, DELETER d) : _m_ptr(ptr), _m_d(d) { }
        protected:
            T *_m_ptr;
            DELETER _m_d;
            virtual void *_get_deleter() {
                return &_m_d;
            }
            virtual void _destroy() {
                _m_d(_m_ptr);
            }
            virtual void _delete_this() {
                delete this;
            }
        }; // _ref_count_del<T, DELETER>

        template <typename T>
        class _ptr_base {
        public:
            typedef T element_type;
            typedef _ptr_base<T> self_type;

            _ptr_base() : _m_ptr(NULL), _m_rep(NULL) { }

#ifdef UNBOOST_RV_REF
            _ptr_base(UNBOOST_RV_REF(self_type) r)
                : _m_ptr(NULL), _m_rep(NULL)
            {
                _assign_rv(forward(UNBOOST_RV(r)));
            }

            template <typename T2>
            _ptr_base(UNBOOST_RV_REF(_ptr_base<T2> ) r) :
                _m_ptr(UNBOOST_RV(r)._m_ptr),
                _m_rep(UNBOOST_RV(r)._m_rep)
            {
                UNBOOST_RV(r)._m_ptr = NULL;
                UNBOOST_RV(r)._m_rep = NULL;
            }

            self_type& operator=(UNBOOST_RV_REF(self_type) r) {
                _assign_rv(forward(UNBOOST_RV(r)));
                return *this;
            }

            void _assign_rv(UNBOOST_RV(self_type) r) {
                if (this != &UNBOOST_RV(r)) {
                    _swap(UNBOOST_RV(r));
                }
            }
#endif  // def UNBOOST_RV_REF

            long _use_count() const {
                return (_m_rep ? _m_rep->_use_count() : 0);
            }

            void _swap(_ptr_base<T>& r) {
                unboost::swap(_m_ptr, r._m_ptr);
                unboost::swap(_m_rep, r._m_rep);
            }

            template <typename T2>
            void _owner_before(const _ptr_base<T2>& r) {
                return _m_rep < r._m_rep;
            }

            void *_get_deleter() {
                return (_m_rep ? _m_rep->_get_deleter() : NULL);
            }

                  T *_get()       { return _m_ptr; }
            const T *_get() const { return _m_ptr; }

            bool _expired() const {
                return (_m_rep == NULL || _m_rep->_expired());
            }

            void _reset() {
                _reset(NULL, NULL);
            }
            template <typename T2>
            void _reset(const _ptr_base<T2>& other) {
                _reset(other._m_ptr, other._m_rep, false);
            }
            template <typename T2>
            void _reset(const _ptr_base<T2>& other, bool does_throw) {
                _reset(other._m_ptr, other._m_rep, does_throw);
            }
            template <typename T2>
            void _reset(const _ptr_base<T2>& other, const _static_tag&) {
                _reset(static_cast<T *>(other._m_ptr), other._m_rep);
            }
            template <typename T2>
            void _reset(const _ptr_base<T2>& other, const _const_tag&) {
                _reset(const_cast<T *>(other._m_ptr), other._m_rep);
            }
            template <typename T2>
            void _reset(const _ptr_base<T2>& other, const _dynamic_tag&) {
                T *ptr = dynamic_cast<T *>(other._m_ptr);
                if (ptr)
                    _reset(ptr, other._m_rep);
                else
                    _reset();
            }
            void _reset(T *other_ptr, _ref_count_base *other_rep) {
                if (other_rep)
                    other_rep->_inc_ref();
                _reset0(other_ptr, other_rep);
            }
            void _reset(T *other_ptr, _ref_count_base *other_rep, bool does_throw) {
                if (other_rep && other_rep->_inc_ref_nz())
                    _reset0(other_ptr, other_rep);
                else if (does_throw)
                    throw bad_weak_ptr();
            }

            void _reset0(T *other_ptr, _ref_count_base *other_rep) {
                if (_m_rep)
                    _m_rep->_dec_ref();
                _m_rep = other_rep;
                _m_ptr = other_ptr;
            }

            void _dec_ref() {
                if (_m_rep)
                    _m_rep->_dec_ref();
            }
            void _dec_wref() {
                if (_m_rep != NULL)
                    _m_rep->_dec_wref();
            }

            void _reset_w() {
                _reset_w((element_type *)NULL, NULL);
            }
            template <typename T2>
            void _reset_w(const _ptr_base<T2>& other) {
                _reset_w(other._m_ptr, other._m_rep);
            }
            template <typename T2>
            void _reset_w(const T2 *other_ptr, _ref_count_base *other_rep) {
                _reset_w(const_cast<T2 *>(other_ptr), other_rep);
            }
            template <typename T2>
            void _reset_w(T2 *other_ptr, _ref_count_base *other_rep) {
                if (other_rep)
                    other_rep->_inc_wref();
                if (_m_rep)
                    _m_rep->_dec_wref();
                _m_rep = other_rep;
                _m_ptr = other_ptr;
            }

        protected:
            T *                 _m_ptr;
            _ref_count_base *   _m_rep;
        }; // _ptr_base<T>

        template <typename T>
        class shared_ptr;
        template <typename T, typename DELETER = default_delete<T> >
        class unique_ptr;
        template <typename T>
        class weak_ptr;

        template <typename T>
        class enable_shared_from_this;

        template <typename T1, typename T2>
        void _do_enable(T1 *ptr, enable_shared_from_this<T2> *es,
                        _ref_count_base *ref_ptr);

        template <typename T>
        void _enable_shared(enable_shared_from_this<T> *ptr,
                            _ref_count_base *ref_ptr);

        void _enable_shared(const volatile void *, const volatile void *);

        template <typename T>
        class shared_ptr : protected _ptr_base<T> {
        public:
            typedef shared_ptr<T> self_type;
            typedef T element_type;
            typedef _ptr_base<T> super_type;

            shared_ptr() { }

            template <typename T2>
            explicit shared_ptr(T2 *ptr) {
                _reset_p(ptr);
            }
            template <typename T2, typename DELETER>
            shared_ptr(T2 *ptr, DELETER d) {
                _reset_p(ptr, d);
            }

            shared_ptr(const self_type& other) {
                this->_reset(other);
            }
            template <typename T2>
            shared_ptr(const shared_ptr<T2>& r, T *ptr) {
                this->_reset(ptr, r);
            }

            template <typename T2>
            shared_ptr(const shared_ptr<T2>& other) {
                this->_reset(other);
            }
            template <typename T2>
            shared_ptr(const weak_ptr<T2>& other, bool does_throw = true) {
                this->_reset(other, does_throw);
            }

            template <typename T2>
            shared_ptr(const shared_ptr<T2>& other, const _static_tag& tag) {
                this->_reset(other, tag);
            }
            template <typename T2>
            shared_ptr(const shared_ptr<T2>& other, const _const_tag& tag) {
                this->_reset(other, tag);
            }
            template <typename T2>
            shared_ptr(const shared_ptr<T2>& other, const _dynamic_tag& tag) {
                this->_reset(other, tag);
            }

#ifdef UNBOOST_RV_REF
            shared_ptr(UNBOOST_RV_REF(self_type) r) :
                super_type(forward(UNBOOST_RV(r))) { }

            self_type& operator=(UNBOOST_RV_REF(self_type) r) {
                self_type(UNBOOST_RV(r)).swap(*this);
                return *this;
            }
            template <typename T2>
            shared_ptr(UNBOOST_RV_REF(shared_ptr<T2>) r) :
                super_type(forward(UNBOOST_RV(r))) { }

            template <typename T2>
            self_type& operator=(UNBOOST_RV_REF(shared_ptr<T2>) r) {
                self_type(UNBOOST_RV(r)).swap(*this);
                return *this;
            }
            #ifdef UNBOOST_OLD_COMPILER
                template <typename T2, typename DELETER>
                self_type& operator=(rv<unique_ptr<T2, DELETER> > r) {
                    self_type(move(UNBOOST_RV(r))).swap(*this);
                    return *this;
                }
            #else
                template <typename T2, typename DELETER>
                self_type& operator=(UNBOOST_RV_REF(unique_ptr<T2, DELETER>) r) {
                    self_type(move(UNBOOST_RV(r))).swap(*this);
                    return *this;
                }
            #endif

            void swap(UNBOOST_RV_REF(self_type) r) {
                super_type::swap(UNBOOST_RV(r));
            }
#endif  // def UNBOOST_RV_REF

            ~shared_ptr() {
                this->_dec_ref();
            }

            self_type operator=(const self_type& r) {
                self_type(r).swap(*this);
                return *this;
            }
            template <typename T2>
            self_type operator=(const shared_ptr<T2>& r) {
                self_type(r).swap(*this);
                return *this;
            }

            void reset() {
                self_type().swap(*this);
            }
            template <typename T2>
            void reset(T2 *ptr) {
                self_type(ptr).swap(*this);
            }
            template <typename T2, typename DELETER>
            void reset(T2 *ptr, DELETER d) {
                self_type(ptr, d).swap(*this);
            }

            void swap(self_type& other) {
                this->_swap(other);
            }

                  T* get()          { return this->_get(); }
            const T* get() const    { return this->_get(); }
            T& operator*() const    { return *this->_get(); }
            T* operator->() const   { return this->_get(); }

            long use_count() const  { return this->_use_count(); }
            bool unique() const     { return (this->_use_count() == 1); }
            operator bool() const   { return get() != NULL; }

            template <typename T2>
            bool owner_before(const shared_ptr<T2>& other) const {
                return this->_owner_before(other);
            }
            template <typename T2>
            bool owner_before(const weak_ptr<T2>& other) const {
                return this->_owner_before(other);
            }

        private:
            template <typename T2>
            void _reset_p(T2 *ptr) {
                _reset_p0(ptr, new _ref_count<T2>(ptr));
            }
            template <typename T2, typename DELETER>
            void _reset_p(T2 *ptr, DELETER d) {
                _reset_p0(ptr, new _ref_count_del<T2, DELETER>(ptr, d));
            }

        public:
            template <typename T2>
            void _reset_p0(T2 *ptr, _ref_count_base *r) {
                this->_reset0(ptr, r);
                _enable_shared(ptr, r);
            }
        }; // shared_ptr<T>

        template <typename DELETER, typename T2>
        inline DELETER* get_deleter(const shared_ptr<T2>& ptr) {
            return (DELETER *)ptr._get_deleter();
        }

        template <typename T1, typename T2>
        inline bool operator==(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) {
            return lhs.get() == rhs.get();
        }
        template <typename T1, typename T2>
        inline bool operator!=(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) {
            return lhs.get() != rhs.get();
        }
        template <typename T1, typename T2>
        inline bool operator<(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) {
            return lhs.get() < rhs.get();
        }
        template <typename T1, typename T2>
        inline bool operator<=(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) {
            return lhs.get() <= rhs.get();
        }
        template <typename T1, typename T2>
        inline bool operator>(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) {
            return lhs.get() > rhs.get();
        }
        template <typename T1, typename T2>
        inline bool operator>=(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) {
            return lhs.get() >= rhs.get();
        }

#ifndef __WATCOMC__
        template <typename T_CHAR, typename T_TRAITS, typename T>
        inline std::basic_ostream<T_CHAR, T_TRAITS>& operator<<(
            std::basic_ostream<T_CHAR, T_TRAITS>& os, const shared_ptr<T>& data)
        {
            os << data.get();
            return os;
        }
#endif

        template <typename T>
        inline void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs) {
            lhs.swap(rhs);
        }

        template <typename T1, typename T2>
        inline shared_ptr<T1> static_pointer_cast(const shared_ptr<T2>& r) {
            return shared_ptr<T1>(r, _static_tag());
        }
        template <typename T1, typename T2>
        inline shared_ptr<T1> const_pointer_cast(const shared_ptr<T2>& r) {
            return shared_ptr<T1>(r, _const_tag());
        }
        template <typename T1, typename T2>
        inline shared_ptr<T1> dynamic_pointer_cast(const shared_ptr<T2>& r) {
            return shared_ptr<T1>(r, _dynamic_tag());
        }

        #define UNBOOST_NEED_UNBOOST_UNIQUE_PTR

        // FIXME: hash

        template <typename T>
        class enable_shared_from_this {
        public:
            typedef enable_shared_from_this<T> self_type;
            typedef T es_type;

            enable_shared_from_this() { }
            enable_shared_from_this(const self_type& obj) { }
            ~enable_shared_from_this() { }
            self_type& operator=(const self_type& obj) { return *this; }
            shared_ptr<T> shared_from_this();
            shared_ptr<T const> shared_from_this() const;

        protected:
            mutable weak_ptr<T> _m_wptr;
        }; // enable_shared_from_this<T>

        template <typename T>
        inline void _enable_shared(
            enable_shared_from_this<T> *ptr, _ref_count_base *ref_ptr)
        {
            if (ptr) {
                _do_enable(ptr, ptr, ref_ptr);
            }
        }

        inline void _enable_shared(const volatile void *,
                                   const volatile void *) { }

        template <typename T>
        class weak_ptr : protected _ptr_base<T> {
        public:
            typedef weak_ptr<T> self_type;
            typedef typename _ptr_base<T>::element_type element_type;

            weak_ptr() { }

            weak_ptr(const self_type& other) {
                this->_reset_w(other);
            }

            template <typename T2>
            weak_ptr(const weak_ptr<T2>& other) {
                this->_reset_w(other);
            }

            template <typename T2>
            weak_ptr(const shared_ptr<T2>& other) {
                this->_reset_w(other);
            }

            ~weak_ptr() {
                this->_dec_wref();
            }

            self_type& operator=(const self_type& r) {
                this->_reset_w(r);
                return *this;
            }

            template <typename T2>
            self_type& operator=(const weak_ptr<T2>& r) {
                this->_reset_w(r);
                return *this;
            }

            template <typename T2>
            self_type& operator=(const shared_ptr<T2>& r) {
                this->_reset_w(r);
                return *this;
            }

            void reset() {
                this->_reset_w();
            }

            void swap(self_type& other) {
                this->_swap(other);
            }

            long use_count() const  { return this->_use_count(); }
            bool expired() const    { return this->_expired(); }

            shared_ptr<T> lock() const {
                return shared_ptr<element_type>(*this, false);
            }

            template <typename T2>
            bool owner_before(const weak_ptr<T2>& other) const {
                return this->_owner_before(other);
            }
            template <typename T2>
            bool owner_before(const shared_ptr<T2>& other) const {
                return this->_owner_before(other);
            }

            template <typename T1, typename T2>
            friend void _do_enable(T1 *ptr, enable_shared_from_this<T2> *es,
                                   _ref_count_base *ref_ptr)
            {
                es->_m_wptr.reset_w(ptr, ref_ptr);
            }
        };

        template <typename T>
        inline void swap(weak_ptr<T>& w1, weak_ptr<T>& w2) {
            w1.swap(w2);
        }

        template <typename T>
        inline shared_ptr<T>
        enable_shared_from_this<T>::shared_from_this() {
            return shared_ptr<T>(_m_wptr);
        }
        template <typename T>
        inline shared_ptr<T const>
        enable_shared_from_this<T>::shared_from_this() const {
            return shared_ptr<T const>(_m_wptr);
        }

        #define UNBOOST_NEED_UNBOOST_MAKE_SHARED
    } // namespace unboost
#else
    #error Your compiler is not supported yet. You lose.
#endif

namespace unboost {
    #ifdef UNBOOST_NEED_UNBOOST_MAKE_SHARED
        template <typename T>
        inline shared_ptr<T> make_shared() {
            shared_ptr<T> ptr(new T());
            return ptr;
        }
        template <typename T, typename T1>
        inline shared_ptr<T> make_shared(const T1& value1) {
            shared_ptr<T> ptr(new T(value1));
            return ptr;
        }
        template <typename T, typename T1, typename T2>
        inline shared_ptr<T> make_shared(const T1& value1, const T2& value2) {
            shared_ptr<T> ptr(new T(value1, value2));
            return ptr;
        }
        template <typename T, typename T1, typename T2, typename T3>
        inline shared_ptr<T> make_shared(const T1& value1, const T2& value2, const T3& value3) {
            shared_ptr<T> ptr(new T(value1, value2, value3));
            return ptr;
        }
        template <typename T, typename T1, typename T2, typename T3, typename T4>
        inline shared_ptr<T> make_shared(const T1& value1, const T2& value2, const T3& value3, const T4& value4) {
            shared_ptr<T> ptr(new T(value1, value2, value3, value4));
            return ptr;
        }
        template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
        inline shared_ptr<T> make_shared(const T1& value1, const T2& value2, const T3& value3, const T4& value4, const T5& value5) {
            shared_ptr<T> ptr(new T(value1, value2, value3, value4, value5));
            return ptr;
        }
    #endif  // def UNBOOST_NEED_UNBOOST_MAKE_SHARED

    #ifdef UNBOOST_NEED_UNBOOST_UNIQUE_PTR
        template <typename T, typename DELETER/* = default_delete<T>*/ >
        class unique_ptr {
        public:
            typedef T *pointer;
            typedef T element_type;
            typedef DELETER deleter_type;
            typedef unique_ptr<T, DELETER> self_type;

            unique_ptr() : m_ptr(NULL) { }

            explicit unique_ptr(pointer ptr) : m_ptr(ptr) { }

            unique_ptr(pointer ptr, DELETER d) : m_ptr(ptr), m_d(d) { }

#ifdef UNBOOST_RV_REF
            unique_ptr(UNBOOST_RV_REF(self_type)& u) :
                m_ptr(UNBOOST_RV(u).m_ptr)
            {
                UNBOOST_RV(u).m_ptr = NULL;
            }

            self_type& operator=(UNBOOST_RV_REF(self_type) r) {
                m_ptr = UNBOOST_RV(r).m_ptr;
                UNBOOST_RV(r).m_ptr = NULL;
                return *this;
            }

            #ifdef UNBOOST_OLD_COMPILER
                template <typename T2, typename D2>
                unique_ptr(rv<unique_ptr<T2, D2> > u) :
                    m_ptr(UNBOOST_RV(u).m_ptr)
                {
                    UNBOOST_RV(u).m_ptr = NULL;
                }

                template <typename T2, typename D2>
                self_type& operator=(rv<unique_ptr<T2, D2> > u) {
                    m_ptr = UNBOOST_RV(u).m_ptr;
                    UNBOOST_RV(u).m_ptr = NULL;
                    return *this;
                }
            #else
                template <typename T2, typename D2>
                unique_ptr(UNBOOST_RV_REF(unique_ptr<T2, D2>) u) :
                    m_ptr(UNBOOST_RV(u).m_ptr)
                {
                    UNBOOST_RV(u).m_ptr = NULL;
                }

                template <typename T2, typename D2>
                self_type& operator=(UNBOOST_RV_REF(unique_ptr<T2, D2>) u) {
                    m_ptr = UNBOOST_RV(u).m_ptr;
                    UNBOOST_RV(u).m_ptr = NULL;
                    return *this;
                }
            #endif
#endif  // def UNBOOST_RV_REF

            ~unique_ptr() {
                pointer ptr = get();
                if (ptr) {
                    get_deleter()(ptr);
                }
            }

            pointer release() {
                pointer ptr = m_ptr;
                m_ptr = NULL;
                return ptr;
            }

            void reset(pointer ptr = pointer()) {
                pointer old_ptr = m_ptr;
                m_ptr = ptr;
                if (old_ptr != NULL)
                    get_deleter()(old_ptr);
            }

            void swap(self_type& ptr) {
                unboost::swap(m_ptr, ptr.m_ptr);
                unboost::swap(m_d, ptr.m_d);
            }

            pointer get() const { return m_ptr; }

                  deleter_type& get_deleter()       { return m_d; }
            const deleter_type& get_deleter() const { return m_d; }

            operator bool() const               { return get() != NULL; }
                  T& operator*()                { return *get(); }
            const T& operator*() const          { return *get(); }
            pointer operator->() const          { return get(); }
                  T& operator[](size_t i)       { return get()[i]; }
            const T& operator[](size_t i) const { return get()[i]; }

        protected:
            T *         m_ptr;
            DELETER     m_d;
        }; // unique_ptr<T, DELETER>

        template <typename T1, typename D1, typename T2, typename D2>
        inline bool operator==(const unique_ptr<T1, D1>& p1, const unique_ptr<T2, D2>& p2) {
            return p1.get() == p2.get();
        }
        template <typename T1, typename D1, typename T2, typename D2>
        inline bool operator!=(const unique_ptr<T1, D1>& p1, const unique_ptr<T2, D2>& p2) {
            return p1.get() != p2.get();
        }
        template <typename T1, typename D1, typename T2, typename D2>
        inline bool operator<(const unique_ptr<T1, D1>& p1, const unique_ptr<T2, D2>& p2) {
            return p1.get() < p2.get();
        }
        template <typename T1, typename D1, typename T2, typename D2>
        inline bool operator<=(const unique_ptr<T1, D1>& p1, const unique_ptr<T2, D2>& p2) {
            return p1.get() <= p2.get();
        }
        template <typename T1, typename D1, typename T2, typename D2>
        inline bool operator>(const unique_ptr<T1, D1>& p1, const unique_ptr<T2, D2>& p2) {
            return p1.get() > p2.get();
        }
        template <typename T1, typename D1, typename T2, typename D2>
        inline bool operator>=(const unique_ptr<T1, D1>& p1, const unique_ptr<T2, D2>& p2) {
            return p1.get() >= p2.get();
        }

        template <typename T, typename D>
        inline void swap(unique_ptr<T, D>& lhs, unique_ptr<T, D>& rhs) {
            lhs.swap(rhs);
        }
    #endif  // def UNBOOST_NEED_UNBOOST_UNIQUE_PTR

    template <typename T>
    struct _default_array_delete {
        typedef _default_array_delete<T> self_type;
        void operator()(T *ptr) {
            if (sizeof(T) > 0) {
                delete[] ptr;
            }
        }
        template <typename T2>
        void operator()(T2 *ptr) { }
    };

    template <typename T, typename DELETER = _default_array_delete<T> >
    class unique_array : public unique_ptr<T, DELETER> {
    public:
        typedef unique_array<T> self_type;
        typedef unique_ptr<T, _default_array_delete<T> > super_type;
        typedef typename super_type::pointer pointer;
        typedef typename super_type::deleter_type deleter_type;

        unique_array() { }
        unique_array(pointer ptr) : super_type(ptr) { }
#ifdef UNBOOST_RV_REF
        unique_array(UNBOOST_RV_REF(super_type) r) :
            super_type(UNBOOST_RV(r)) { }
        self_type& operator=(UNBOOST_RV_REF(self_type) r) {
            super_type::operator=(UNBOOST_RV(r));
            return *this;
        }
#endif
        pointer release() {
            return super_type::release();
        }
        void reset(pointer ptr = pointer()) {
            super_type::reset(ptr);
        }
        void swap(self_type& other) {
            super_type::swap(other);
        }
        pointer get() {
            return super_type::get();
        }
        deleter_type& get_deleter() {
            return super_type::get_deleter();
        }
        const deleter_type& get_deleter() const {
            return super_type::get_deleter();
        }

        operator bool() const {
            return super_type::operator bool();
        }
        T& operator*() {
            return super_type::operator*();
        }
        const T& operator*() const {
            return super_type::operator*();
        }
        pointer operator->() const {
            return super_type::operator->();
        }

              T& operator[](size_t i)       { return get()[i]; }
        const T& operator[](size_t i) const { return get()[i]; }
    }; // unique_array<T, DELETER>
} // namespace unboost

#endif  // ndef UNBOOST_SMART_PTR_HPP_
