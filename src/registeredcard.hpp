#ifndef __REGISTEREDCARD_HPP
#define __REGISTEREDCARD_HPP
#pragma interface

class PUDPSocket;

class RegisteredCard {
public:
	bool h323, fromNat;
	ss :: string ip, rip; // rip - RAS IP. Redundant?
	unsigned short port, rport;
	StringSet contactList;
	PUDPSocket * rasSock;
	RegisteredCard ( bool h = true ) : h323 ( h ), fromNat ( false ), port ( 0 ), rport ( 0 ), rasSock ( 0 ) { }
	RegisteredCard ( bool h, PUDPSocket * s, const ss :: string & i, unsigned short p, const ss :: string & ri /*= ""*/,
		unsigned short rp /*= 0*/, bool fn /*= false*/ ) : h323 ( h ), fromNat ( fn ), ip ( i ), rip ( ri ), port ( p ),
		rport ( rp ), rasSock ( s ) { }
};

typedef std :: map < ss :: string, RegisteredCard, std :: less < ss :: string >, __SS_ALLOCATOR < std :: pair < const ss :: string, RegisteredCard > > > RegCardsMap;

inline std :: ostream & operator<< ( std :: ostream & os, const RegisteredCard & rc ) {
	return os << ( rc.h323 ? "h323" : "sip" ) << ", " << ( rc.fromNat ? "" : "not " ) << "from nat, ip: " << rc.ip <<
		", rip: " << rc.rip << ", port: " << rc.port << ", rport: " << rc.rport;
}
#endif
