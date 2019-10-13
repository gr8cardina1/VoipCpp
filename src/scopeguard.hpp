#ifndef SCOPEGUARD_HPP_
#define SCOPEGUARD_HPP_
#pragma interface

#include "nonaliasable.hpp"
class nonheapallocatable {
	static void * operator new ( std :: size_t );
	static void * operator new[] ( std :: size_t );
	static void operator delete ( void *, std :: size_t ) throw ( );
	static void operator delete[] ( void *, std :: size_t ) throw ( );
};
class ScopeGuardBase : nonheapallocatable, nonaliasable {
protected:
	mutable bool dismissed;
	ScopeGuardBase ( ) : dismissed ( false ) { }
	ScopeGuardBase ( const ScopeGuardBase & other ) : dismissed ( other.dismissed ) {
		other.dismiss ( );
	}
	~ScopeGuardBase ( ) { }
public:
	void dismiss ( ) const {
		dismissed = true;
	}
};
template < typename T > class ScopeGuard2 : public ScopeGuardBase {
	T f;
public:
	ScopeGuard2 ( const T & t ) : f ( t ) { }
	~ScopeGuard2 ( ) {
		if ( ! dismissed ) {
			try {
				f ( );
			} catch ( ... ) { }
		}
	}
};
template < class T > class ScopeGuard2 < T ( ) > : public ScopeGuardBase {
	T ( * f ) ( );
public:
	ScopeGuard2 ( T ( * t ) ( ) ) : f ( t ) { }
	~ScopeGuard2 ( ) {
		if ( ! dismissed ) {
			try {
				f ( );
			} catch ( ... ) { }
		}
	}
};
typedef const ScopeGuardBase & __attribute__ ( ( unused ) ) ScopeGuard;
template < typename T > const ScopeGuard2 < T > makeGuard ( const T & t ) {
	return ScopeGuard2 < T > ( t );
}

#endif /*SCOPEGUARD_HPP_*/
