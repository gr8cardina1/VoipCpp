#include "ss.hpp"
#include "translateipmask.hpp"
#include <ptlib.h>
#include <ptlib/svcproc.h>
#include <arpa/inet.h>
#include <limits>

StringVector translateIpMask ( const ss :: string & ip ) {
	StringVector r;
	ss :: string :: size_type slash = ip.find ( '/' );
	if ( slash == ss :: string :: npos ) {
		r.push_back ( ip );
		return r;
	}
	in_addr a;
	if ( inet_pton ( AF_INET, ip.substr ( 0, slash ).c_str ( ), & a ) <= 0 ) {
		PSYSTEMLOG ( Error, "bad ip address: " << ip );
		return r;
	}
	unsigned nbits = 32;
	if ( ip.find ( '.', slash + 1 ) == ss :: string :: npos )
		nbits -= std :: atoi ( ip.substr ( slash + 1 ).c_str ( ) );
	else {
		in_addr a;
		if ( inet_pton ( AF_INET, ip.substr ( slash + 1 ).c_str ( ), & a ) <= 0 ) {
			PSYSTEMLOG ( Error, "bad ip address network: " << ip );
			return r;
		}
		nbits = __builtin_clz ( ntohl ( a.s_addr ) );
	}
	if ( nbits > 16 ) {
		PSYSTEMLOG ( Error, "ip range is too big: " << ip );
		return r;
	}
	a.s_addr = htonl ( ntohl ( a.s_addr ) & ( std :: numeric_limits < unsigned > :: max ( ) << nbits ) );
	unsigned cnt = 1 << nbits;
	r.reserve ( cnt );
	for ( unsigned i = 0; i < cnt; i ++ ) {
		char buf [ INET_ADDRSTRLEN ];
		r.push_back ( inet_ntop ( AF_INET, & a, buf, INET_ADDRSTRLEN ) );
		a.s_addr = htonl ( ntohl ( a.s_addr ) + 1 );
	}
	return r;
}

