#include <cstring>
#include <unistd.h>
#include <iostream>
#include <cerrno>
#include <sstream>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdexcept>
#include <iomanip>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "../ss.hpp"
#include "../allocatable.hpp"
#include "../pointer.hpp"
#include "../radius.hpp"

class ExternalRoute {
	ss :: string secret;
	ss :: string ani;
	IntVector routes;
	int fd;
	enum {
		bufLen = 4096
	};
	char buf [ bufLen ];
	int len;
	sockaddr_in fromAddr;
	int sl;
	bool makeSocket ( );
	void makeAnswer ( );
	public:
	ExternalRoute ( const ss :: string & a, const IntVector & r );
	bool run ( );
};

ExternalRoute :: ExternalRoute ( const ss :: string & a, const IntVector & r ) : secret ( "secret" ), ani ( a ),
	routes ( r ), fd ( - 1 ), sl ( sizeof ( fromAddr ) ) { }

void ExternalRoute :: makeAnswer ( ) {
	using namespace Radius;
	char t [ 16 ];
	Pointer < Request > r = new Request ( secret, inet_ntop ( AF_INET, & fromAddr.sin_addr.s_addr, t, 16 ),
		ntohs ( fromAddr.sin_port ), reinterpret_cast < unsigned char * > ( buf ), len );
	Request answer ( * r, rAuthenticationAck );
	answer.append ( new H323IvrIn ( "ReplaceANI:" + ani ) );
	for ( int i = 0; i < routes.size ( ); i ++ ) {
		ss :: ostringstream os;
		os << "Outbound:" << routes [ i ];
		answer.append ( new H323IvrIn ( os.str ( ) ) );
	}
	len = answer.print ( buf, bufLen );
}
	
bool ExternalRoute :: makeSocket ( ) {
	fd = socket ( AF_INET, SOCK_DGRAM, 0 );
	if ( fd == - 1 ) {
		std :: cerr << "socket(): " << strerror ( errno ) << std :: endl;
		return false;
	}
	int cmd = 1;
//	if ( ioctl ( fd, FIONBIO, & cmd ) ) {
//		std :: cerr << "FIONBIO: " << strerror ( errno ) << std :: endl;
//		close ( fd );
//		return false;
//	}
	if ( fcntl ( fd, F_SETFD, FD_CLOEXEC ) == - 1 ) {
		std :: cerr << "F_SETFD: " << strerror ( errno ) << std :: endl;
		close ( fd );
		return false;
	}
	cmd = IPTOS_LOWDELAY;
	if ( geteuid ( ) == 0 )
		cmd |= IPTOS_PREC_CRITIC_ECP;
	if ( setsockopt ( fd, IPPROTO_IP, IP_TOS, & cmd, sizeof ( cmd ) ) ) {
		std :: cerr << "IP_TOS: " << strerror ( errno ) << std :: endl;
		close ( fd );
		return false;
	}
	sockaddr_in sin;
	std :: memset ( & sin, 0, sizeof ( sin ) );
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons ( 1812 );
	if ( bind ( fd, ( struct sockaddr * ) & sin, sizeof ( sin ) ) ) {
		std :: cerr << "bind: " << strerror ( errno ) << std :: endl;
		return false;
	}
	return true;
}

bool ExternalRoute :: run ( ) {
	if ( ! makeSocket ( ) )
		return false;
	while ( true ) {
		sockaddr_in fromAddr;
		socklen_t sl = sizeof ( fromAddr );
		len = recvfrom ( fd, buf, bufLen, 0, reinterpret_cast < sockaddr * > ( & fromAddr ), & sl );
		if ( len == -1 ) {
			std :: cerr << "Read: " << strerror ( errno ) << std :: endl;
			return false;
		}
		if ( len == 0 ) {
			std :: cerr << "Read: nothing to read" << std :: endl;
			return false;
		}
		if ( len == bufLen )
			std :: cerr << "Reading buffer filled" << std :: endl;
		makeAnswer ( );
		int r = sendto ( fd, buf, len, 0, reinterpret_cast < sockaddr * > ( & fromAddr ), sl );
		if ( r == len )
			continue;;
		switch ( r ) {
			case - 1:
				std :: cerr << "sendto: " << strerror ( errno ) << std :: endl;
				return false;
			case 0:
				std :: cerr << "sendto: can't write" << std :: endl;
				return false;
		}
		std :: cerr << "sendto: can't write " << len << " bytes, only " << r << std :: endl;
		return false;
	}
}
		

int main ( int argc, char * * argv ) {
	if ( argc < 3 ) {
		std :: cerr << "usage: " << argv [ 0 ] << " ani outpeer1 outpeer2 ..." << std :: endl;
		return EXIT_FAILURE;
	}
	IntVector routes;
	for ( int i = 2; i < argc; i ++ )
		routes.push_back ( atoi ( argv [ i ] ) );
	ExternalRoute er ( argv [ 1 ], routes );
	er.run ( );
}
