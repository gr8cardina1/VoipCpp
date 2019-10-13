#ifndef __ALLOCATABLE_HPP
#define __ALLOCATABLE_HPP
#pragma interface

template < template < typename > class _Allocator > class Allocatable {
protected:
	~Allocatable ( ) { }
	Allocatable ( ) { }
	Allocatable ( const Allocatable & ) { }
public:
	static void * operator new ( std :: size_t nSize ) {
		return _Allocator < char > ( ).allocate ( nSize );
	}
	static void * operator new ( std :: size_t /*nSize*/, void * p ) {
		return p;
	}
	static void * operator new[] ( std :: size_t nSize ) {
		return _Allocator < char > ( ).allocate ( nSize );
	}
	static void operator delete ( void * ptr, std :: size_t s ) throw ( ) {
		_Allocator < char > ( ).deallocate ( static_cast < char * > ( ptr ), s );
	}
	static void operator delete[] ( void * ptr, std :: size_t s ) throw ( ) {
		_Allocator < char > ( ).deallocate ( static_cast < char * > ( ptr ), s );
	}
};
#endif
