#ifndef __FIRSTITERATOR_HPP
#define __FIRSTITERATOR_HPP
#pragma interface

template < typename _Iterator > class first_iterator :
	public std :: iterator < typename std :: iterator_traits < _Iterator > :: iterator_category,
	typename std :: iterator_traits < _Iterator > :: value_type :: first_type,
	typename std :: iterator_traits < _Iterator > :: difference_type,
	typename std :: iterator_traits < _Iterator > :: value_type :: first_type *,
	typename std :: iterator_traits < _Iterator > :: value_type :: first_type & > {
protected:
	_Iterator current;
public:
	typedef _Iterator iterator_type;
	typedef typename std :: iterator_traits < _Iterator > :: difference_type difference_type;
	typedef typename std :: iterator_traits < _Iterator > :: value_type :: first_type & reference;
	typedef typename std :: iterator_traits < _Iterator > :: value_type :: first_type * pointer;
public:
	first_iterator ( ) : current ( ) { }
	explicit first_iterator ( iterator_type __x ) : current ( __x ) { }
	first_iterator ( const first_iterator & __x) : current ( __x.current ) { }
	template < typename _Iter > first_iterator ( const first_iterator < _Iter > & __x ) : current ( __x.base ( ) ) { }
	iterator_type base ( ) const {
		return current;
	}
	reference operator* ( ) const {
		return ( * current ).first;
	}
	pointer operator-> ( ) const {
		return & ( operator* ( ) );
	}
	first_iterator & operator++ ( ) {
		++ current;
		return * this;
	}
	first_iterator operator++ ( int ) {
		first_iterator __tmp = * this;
		++ current;
		return __tmp;
	}
	first_iterator & operator-- ( ) {
		-- current;
		return * this;
	}
	first_iterator operator-- ( int ) {
		first_iterator __tmp = * this;
		-- current;
		return __tmp;
	}
	first_iterator operator+ ( difference_type __n ) const {
		return first_iterator ( current + __n );
	}
	first_iterator & operator+= ( difference_type __n ) {
		current += __n;
		return * this;
	}
	first_iterator operator- ( difference_type __n ) const {
		return first_iterator ( current - __n );
	}
	first_iterator & operator-= ( difference_type __n ) {
		current -= __n;
		return * this;
	}
	reference operator[] ( difference_type __n ) const {
		return * ( * this + __n );
	}
};

template < typename _Iterator > inline bool operator== ( const first_iterator < _Iterator > & __x,
	const first_iterator < _Iterator > & __y ) {
	return __x.base ( ) == __y.base ( );
}

template < typename _Iterator > inline bool operator< ( const first_iterator < _Iterator> & __x,
	const first_iterator < _Iterator > & __y ) {
	return __x.base ( ) < __y.base ( );
}

template < typename _Iterator > inline bool operator!= ( const first_iterator < _Iterator > & __x,
	const first_iterator < _Iterator > & __y ) {
	return ! (__x == __y );
}

template < typename _Iterator > inline bool operator> ( const first_iterator < _Iterator > & __x,
	const first_iterator < _Iterator > & __y ) {
	return __y < __x;
}

template < typename _Iterator > inline bool operator<= ( const first_iterator < _Iterator > & __x,
	const first_iterator < _Iterator > & __y ) {
	return ! (__y < __x );
}

template < typename _Iterator > inline bool operator>= ( const first_iterator < _Iterator > & __x,
	const first_iterator < _Iterator > & __y ) {
	return ! (__x < __y );
}

template < typename _Iterator > inline typename first_iterator < _Iterator > :: difference_type
	operator- ( const first_iterator < _Iterator > & __x, const first_iterator < _Iterator > & __y ) {
	return __x.base ( ) - __y.base ( );
}

template < typename _Iterator > inline first_iterator < _Iterator > operator+ (
	typename first_iterator < _Iterator > :: difference_type __n, const first_iterator < _Iterator > & __x ) {
	return first_iterator < _Iterator > ( __x.base ( ) + __n );
}

template < typename _Iter > inline first_iterator < _Iter > firster ( const _Iter & __x ) {
	return first_iterator < _Iter > ( __x );
}

#endif
