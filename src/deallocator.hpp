#ifndef DEALLOCATOR_HPP_
#define DEALLOCATOR_HPP_
#pragma interface

template < typename T > class Deallocator {
	typedef __SS_ALLOCATOR < T > A;
	typedef typename A :: size_type size_type;
	T * p;
	size_type sz;
public:
	Deallocator ( T * pp, size_type s ) : p ( pp ), sz ( s ) { }
	void operator() ( ) {
		A ( ).deallocate ( p, sz );
	}
};

#endif /*DEALLOCATOR_HPP_*/
