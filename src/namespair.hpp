#ifndef NAMESPAIR_HPP_
#define NAMESPAIR_HPP_
#pragma interface

struct NamesPair {
	unsigned first;
	const char * second;
	template < typename U > operator std :: pair < const U, ss :: string > ( ) const {
		return std :: pair < const U, ss :: string > ( first, second );
	}
};

typedef std :: map < unsigned, ss :: string, std :: less < unsigned >,
	__SS_ALLOCATOR < std :: pair < const unsigned, ss :: string > > > NamesMapType;

#endif /*NAMESPAIR_HPP_*/
