#ifndef MATH_VECTOR_HPP
#define MATH_VECTOR_HPP
 
/*
 *  (C) Copyright Scott McMurray 2006-2007. 
 *
 *  Use, modification and distribution are subject to the 
 *  Boost Software License, Version 1.0. (See accompanying file 
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 *  The most recent version of this class can be found online at
 *  http://gpwiki.org/index.php?title=C_plus_plus:Tutorials:TemplateVector
 */
 
#include <cstddef>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cmath>
#include <cassert>
 
#ifndef MATH_VECTOR_NO_IO
#include <istream>
#include <ostream>
#endif
 
#ifdef BOOST_PREVENT_MACRO_SUBSTITUTION
# ifndef USE_BOOST
#  ifndef NO_USE_BOOST
#   define USE_BOOST 1
#  endif
# else
#  ifdef NO_USE_BOOST
#   undef USE_BOOST
#  endif
# endif
#endif
 
#ifdef USE_BOOST
#include <boost/call_traits.hpp>
#endif
 
#ifdef USE_BOOST
#include <boost/static_assert.hpp>
#else
#ifndef BOOST_STATIC_ASSERT
#define BOOST_STATIC_ASSERT(c) typedef int ASSERTION_CHECK[(c)?1:-1]
#endif
#endif

#undef max
#undef min

namespace math {
 
template < std::size_t N = 3, typename T = float >
class vector {
#ifdef MATH_VECTOR_AGGREGATE
  public:
#endif
    T v_[N];
  public:
    typedef T value_type;
    typedef value_type const const_value_type;
 
    typedef value_type& reference;
    typedef const_value_type& const_reference;
 
    typedef value_type* pointer;
    typedef const_value_type* const_pointer;
 
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
 
#ifdef USE_BOOST
    typedef typename boost::call_traits<value_type>::param_type parameter_type;
#else
//    typedef const_reference parameter_type;
    typedef const_value_type parameter_type;
#endif
 
    template <typename U>
    vector &operator=(U (&a)[N]) {
        std::copy( a, a+N, v_ );
        return *this;
    }
    template <typename U>
    vector &operator=(vector<N,U> const &v) {
        std::copy( v.begin(), v.begin()+N, v_ );
        return *this;
    }

    template <typename U>
    operator vector<N,U>() const {
        vector<N,U> ret;
        ret = *this;
        return ret;
    }
#ifndef MATH_VECTOR_AGGREGATE
    vector() {
        std::fill_n( v_, N, value_type() );
    }
    template <typename U>
    /* implicit */ vector(U (&a)[N]) {
        *this = a;
    }
    explicit vector(parameter_type x, size_type n = N) {
        if ( n > N ) n = N;
        std::fill( v_, v_+n, x );
        std::fill( v_+n, v_+N, value_type() );
    }
    vector(parameter_type x, parameter_type y) {
        BOOST_STATIC_ASSERT(N==2);
        v_[0] = x;
        v_[1] = y;
    }
    vector(parameter_type x, parameter_type y, parameter_type z) {
        BOOST_STATIC_ASSERT(N==3);
        v_[0] = x;
        v_[1] = y;
        v_[2] = z;
    }
    vector(parameter_type x, parameter_type y, parameter_type z, parameter_type w) {
        BOOST_STATIC_ASSERT(N==4);
        v_[0] = x;
        v_[1] = y;
        v_[2] = z;
        v_[3] = w;
    }
    explicit vector(const_pointer p, size_type n = N) {
        if ( n > N ) n = N;
        std::copy( p, p+n, v_ );
        std::fill_n( v_+n, N-n, value_type() );
    }
    template <typename U>
    explicit vector(vector<N,U> const &v) {
        *this = v;
    }
#endif
 
    const_pointer v() const {
        return v_;
    }
    pointer v() {
        return const_cast<pointer>(
                const_cast<vector const &>(*this).v()
               );
    }
 
    template <size_type i>
    const_reference get() const { 
        BOOST_STATIC_ASSERT(i < N);
        return v_[i];
    }
    template <size_type i>
    reference get() { 
        return const_cast<reference>(
                const_cast<vector const &>(*this).get<i>()
               );
    }
 
    const_reference x() const { return get<0>(); }
    reference x() { return get<0>(); }
    const_reference y() const { return get<1>(); }
    reference y() { return get<1>(); }
    const_reference z() const { return get<2>(); }
    reference z() { return get<2>(); }
    const_reference w() const { return get<3>(); }
    reference w() { return get<3>(); }
 
    pointer begin() { return v_; }
    const_pointer begin() const { return v_; }
    pointer end() { return v_+N; }
    const_pointer end() const { return v_+N; }
    static size_type size() { return N; }
    // N==0 is impossible, since it would mean a 0-length array
    // but 0-length arrays may be possible in the future, so be safe
    static bool empty() { return N != 0; }
 
#ifdef MATH_VECTOR_NO_IMPLICIT_CONVERSION
    const_reference operator*() const { return x(); }
    reference operator*() { return x(); }
 
    const_reference operator[](size_type i) const {
        assert( i < N || !"=> index out of range" );
        return v_[i];
    }
    reference operator[](size_type i) {
        return const_cast<reference>(
                const_cast<vector const &>(*this).operator[](i)
               );
    }
#else 
    operator pointer() { return v(); }
    operator const_pointer() const { return v(); }
#endif
 
    vector &operator+=(vector const &vec) {
        std::transform( v(), v()+size(),
                        vec.v(),
                        v(),
                        std::plus<value_type>() );
        return *this;
    }
    vector &operator-=(vector const &vec) {
        std::transform( v(), v()+size(),
                        vec.v(),
                        v(),
                        std::minus<value_type>() );
        return *this;
    }
 
    vector &operator*=(parameter_type c) {
        std::transform( v(), v()+size(),
                        v(),
                        std::bind2nd( std::multiplies<value_type>(), c ) );
        return *this;
    }
    vector &operator/=(parameter_type c) {
        std::transform( v(), v()+size(),
                        v(),
                        std::bind2nd( std::divides<value_type>(), c ) );
        return *this;
    }
 
};
 
template < std::size_t N, typename T, typename U >
bool operator==(vector<N,T> const &lhs, vector<N,U> const &rhs) {
    return std::equal( lhs.v(), lhs.v()+N,
                       rhs.v() );
}
template < std::size_t N, typename T, typename U >
bool operator!=(vector<N,T> const &lhs, vector<N,U> const &rhs) {
    return !( lhs == rhs );
}
 
template < std::size_t N, typename T >
vector<N,T> operator+(vector<N,T> lhs, vector<N,T> const &rhs) {
    return lhs += rhs;
}
template < std::size_t N, typename T >
vector<N,T> operator-(vector<N,T> lhs, vector<N,T> const &rhs) {
    return lhs -= rhs;
}
 
template < std::size_t N, typename T >
vector<N,T> operator*(vector<N,T> vec,
                      typename vector<N,T>::parameter_type c) {
    return vec*=c;
}
template < std::size_t N, typename T >
vector<N,T> operator*(typename vector<N,T>::parameter_type c,
                      vector<N,T> vec) {
    return vec*=c;
}
template < std::size_t N, typename T >
vector<N,T> operator/(vector<N,T> vec,
                      typename vector<N,T>::parameter_type c) {
    return vec/=c;
}
 
template < std::size_t N, typename T >
vector<N,T> operator-(vector<N,T> vec) {
    std::transform( vec.begin(), vec.end(), 
                    vec.begin(),
                    std::negate<T>() );
    return vec;
}
 
template < std::size_t N, typename T >
vector<N,T> const &operator+(vector<N,T> const &vec) {
    return vec;
}
 
template < std::size_t N, typename T >
T dot(vector<N,T> lhs, vector<N,T> const &rhs) {
    return std::inner_product( lhs.v(), lhs.v()+N,
                               rhs.v(),
                               static_cast<T>(0) );
}
 
template < typename T >
T cross(vector<2,T> const &lhs, vector<2,T> const &rhs) {
    return lhs[0]*rhs[1]-lhs[1]*rhs[0];
}
template < typename T >
vector<3,T> cross(vector<3,T> const &lhs, vector<3,T> const &rhs) {
#ifndef MATH_VECTOR_AGGREGATE
    return vector<3,T>( lhs[1]*rhs[2]-lhs[2]*rhs[1],
                        lhs[2]*rhs[0]-lhs[0]*rhs[2],
                        lhs[0]*rhs[1]-lhs[1]*rhs[0] );
#else
    vector<3,T> const ret = { lhs[1]*rhs[2]-lhs[2]*rhs[1],
                              lhs[2]*rhs[0]-lhs[0]*rhs[2],
                              lhs[0]*rhs[1]-lhs[1]*rhs[0] };
    return ret;
#endif
}
 
template < std::size_t N, std::size_t M, typename T >
vector<N,vector<M,T> > outer(vector<N,T> lhs, vector<M,T> const &rhs) {
#ifndef MATH_VECTOR_AGGREGATE
    vector<N,vector<M,T> > ret(N, rhs);
#else
    vector<N,vector<M,T> > ret;
    std::fill( ret.begin(), ret.end(), rhs );
#endif
    for (unsigned i = 0; i < N; ++i) ret[i] *= lhs[i];
    return ret;
}

template < std::size_t N, typename T >
T norm(vector<N,T> const &vec) {
    return dot( vec, vec );
}
 
template < std::size_t N, typename T >
T length(vector<N,T> const &vec) {
    // this uses a using declaration instead of explicitly qualifying the call
    // to avoid interfering with argument-dependent lookup ( Koenig Lookup )
    using std::sqrt;
    return sqrt( norm(vec) );
}

template < std::size_t N, typename T >
vector<N,T> unit(vector<N,T> const &vec) {
    return vec / length(vec);
}
 
template < std::size_t N, typename T >
T manhatten(vector<N,T> const &vec) {
    T accum = T();
    // this uses a using declaration instead of explicitly qualifying the call
    // to avoid interfering with argument-dependent lookup ( Koenig Lookup )
    using std::abs;
    for (unsigned i = 0; i < N; ++i) accum += abs(vec[i]);
    return accum;
}

template < std::size_t N, typename T >
T cos(vector<N,T> const &a, vector<N,T> const &b) {
    using std::sqrt;
    return dot(a,b)/sqrt(norm(a)*norm(b));
}

#ifndef MATH_VECTOR_NO_IO
template < std::size_t N, typename T >
std::ostream &operator<<(std::ostream &sink, vector<N,T> const &vec) {
    sink << "( ";
    std::copy( vec.v(), vec.v()+N-1,
               std::ostream_iterator<T>( sink, ", " ) );
    sink << vec[N-1] << " )";
    return sink;
}
 
template < std::size_t N, typename T >
std::istream &operator>>(std::istream &source, vector<N,T> &vec) {
    if ( !source.good() ) return source;
 
    char c = '\0';
    source >> c;
    if ( c != '(' ) {
        source.unget();
        source.clear( std::ios::failbit );
        return source;
    }
 
    for ( std::size_t i = 0; i < N; ++i ) {
 
        source >> vec[i];
        source.clear( source.rdstate() & ~std::ios::failbit );
        if ( !source.good() ) return source;
 
        c = '\0';
        source >> c;
        source.clear( source.rdstate() & ~std::ios::failbit );
        if ( c != ( (i == N-1) ? ')' : ',' ) ) {
            source.unget();
            source.clear( std::ios::failbit );
            return source;
        }
        if ( !source.good() ) return source;
 
    }
 
    return source;
}
#endif
 
} // namespace math
 
namespace std {

template < std::size_t N, typename T >
struct less< math::vector<N,T> > {
    bool operator()(math::vector<N,T> const &lhs,
                    math::vector<N,T> const &rhs) const {
        return lexicographical_compare( lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end() ); 
    }
};

} // namespace std 
 
#ifndef MATH_VECTOR_NO_LIMITS_SPECIALIZATION
#include <limits>
namespace std {

template < std::size_t N, typename T >
struct numeric_limits< math::vector<N,T> > {
	static const bool is_specialized = true;
	static math::vector<N,T> max() {
#ifndef MATH_VECTOR_AGGREGATE
    	return math::vector<N,T>( N, numeric_limits<T>::max() );
#else
    	math::vector<N,T> ret;
	    std::fill( ret.begin(), ret.end(), numeric_limits<T>::max() );
		return ret;
#endif
	}
	static math::vector<N,T> min() {
#ifndef MATH_VECTOR_AGGREGATE
    	return math::vector<N,T>( N, numeric_limits<T>::min() );
#else
    	math::vector<N,T> ret;
	    std::fill( ret.begin(), ret.end(), numeric_limits<T>::min() );
		return ret;
#endif
	}
};

} // namespace std
 
#endif 
 
#endif 
