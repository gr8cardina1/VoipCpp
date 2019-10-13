#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <iomanip>
#include <stdexcept>
#include <limits>
#include <cstring>
#include "asn.hpp"
#include "h225.hpp"
#include "h235_t.cxx" //nado avtomatom instantiate templates v asnparsere
			//ili prosto generit _t v konec .hpp, blago on ne bolshoy
#include <ptlib.h>
#include <ptlib/ipsock.h>
#include "AddrUtils.h"

template class std :: basic_string < unsigned short, std :: char_traits < unsigned short >, __SS_ALLOCATOR < unsigned short > >;
template class std :: basic_string < char, std :: char_traits < char >, __SS_ALLOCATOR < char > >;

void AddrUtils :: getIPAddress ( H225 :: TransportAddress & ipAddress,
	const H225 :: ArrayOf_TransportAddress & addresses ) {
// Task: given an array of transport address, returns the first IP address
// will throw an error if no IP address
	for ( std :: size_t i = 0, sz = addresses.size ( ); i < sz; ++ i ) {
		if ( addresses [ i ].getTag ( ) == H225 :: TransportAddress :: e_ipAddress ) {
			ipAddress = addresses [ i ];
			return;
		}
	}
	throw IPAddrNotFoundError ( "No H225 IP address found!" );
}

void AddrUtils :: getIPAddress ( H225 :: TransportAddress_ipAddress & ipAddress,
	const H225 :: ArrayOf_TransportAddress & addresses ) {
// Task: given an array of transport address, returns the first IP address
// will throw an error if no IP address
	H225 :: TransportAddress akaIPAddr;
	AddrUtils :: getIPAddress ( akaIPAddr, addresses );
	ipAddress = static_cast < H225 :: TransportAddress_ipAddress & > ( akaIPAddr );
}

H225 :: TransportAddress AddrUtils :: convertToH225TransportAddr ( const PIPSocket :: Address & addr,
	WORD port ) {
// Task: to convert an IP address into an H225 transport address
	H225 :: TransportAddress result;
	result.setTag ( H225 :: TransportAddress :: e_ipAddress );
	H225 :: TransportAddress_ipAddress & resultIP = result;

	Asn :: string t;
	t.push_back ( addr.Byte1 ( ) );
	t.push_back ( addr.Byte2 ( ) );
	t.push_back ( addr.Byte3 ( ) );
	t.push_back ( addr.Byte4 ( ) );
	resultIP.m_ip = t;
	resultIP.m_port = port;

	return result;
}

void AddrUtils :: convertToIPAddress ( const H225 :: TransportAddress_ipAddress & h225ip,
	PIPSocket :: Address & addr, WORD & port ) {
// Task: to convert an IP address in H225 format to PWLIB format
	const Asn :: string & t = h225ip.m_ip;
	PIPSocket :: Address resultAddr ( t [ 0 ], t [ 1 ], t [ 2 ], t [ 3 ] );
	addr = resultAddr;
	port = static_cast < WORD > ( h225ip.m_port );
}
