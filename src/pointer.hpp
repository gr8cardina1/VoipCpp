#ifndef __POINTER_HPP
#define __POINTER_HPP
#pragma interface

class PointRefBase : public Allocatable < __SS_ALLOCATOR > {
	int c;
public:
	PointRefBase ( ) : c ( 1 ) { }
	void ref ( ) {
		c ++;
	}
	bool unRef ( ) {
		return -- c <= 0;
	}
protected:
	~PointRefBase ( ) { }
};
template < class T > class PointRef : public PointRefBase {
	T * p;
	short d;
	PointRef & operator = ( const PointRef & );
	PointRef ( const PointRef & );
public:
	PointRef ( T * t, int D ) : p ( t ), d ( short ( D ) ) { }
	~PointRef ( ) {
		switch ( d ) {
			case 1:
				delete p;
				break;
			case 2:
				delete [ ] p;
		}
	}
	T * data ( ) const {
		return p;
	}
};

template < class T > class Pointer {
	PointRef < T > * r;
public:
	enum delOnExit {
		notDel,
		del,
		delArray
	};
	Pointer ( ) : r ( 0 ) { }
	Pointer ( T * t, delOnExit d = del ) {
		try {
			r = new PointRef < T > ( t, d );
		} catch ( ... ) {
			 PointRef < T > ( t, d ); //delete t
			 throw;
		}
	}
	Pointer ( const Pointer & p ) : r ( p.r ) {
		if ( r )
			r -> ref ( );
	}
	~Pointer ( ) {
		if ( r && r -> unRef ( ) )
			delete r;
	}
	operator T * ( ) const {
		return r ? r -> data ( ) : 0;
	}
	Pointer & operator = ( const Pointer & p ) {
		if ( this != & p ) {
			if ( r && r -> unRef ( ) )
				delete r;
			if ( ( r = p.r ) )
				r -> ref ( );
		}
		return * this;
	}
	Pointer & operator = ( T * p ) {
		PointRef < T > * t;
		try {
			t = new PointRef < T > ( p, del );
		} catch ( ... ) {
			delete p;
			throw;
		}
		if ( r && r -> unRef ( ) )
			delete r;
		r = t;
		return * this;
	}
	T * operator -> ( ) const {
		return operator T * ( );
	}
	T * data ( ) const {
		return operator T * ( );
	}
};

template < > inline PointRef < void > :: ~PointRef ( ) {
	delete reinterpret_cast < char * > ( p );
}

template < > inline PointRef < const void > :: ~PointRef ( ) {
	delete reinterpret_cast < const char * > ( p );
}

template < class T > void safeDel ( T * & t ) {
	delete t;
	t = 0;
}
#endif
/*
typedef Pointer < int > in;
#include <iostream.h>
int main ( ) {
	in a ( new int ( 5 ) );
	in b ( a );
	in c;
	c = a;
	in d ( new int [ 10 ], in :: delArray );
	cout << "a: " << * a << " b: " << * b << " c: " << * c
		<< " d: " << d [ 0 ] << endl;
}
*/
