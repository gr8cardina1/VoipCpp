
#include "../ss.hpp"
#include "../allocatable.hpp"
#include "../rtpstat.hpp"

#include "rtpprocess.hpp"
#include <cstring>
#include "rtprequest.hpp"
#include <unistd.h>
#include <iostream>
#include <cerrno>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdexcept>
#include <iomanip>
#include <sys/time.h>
#include "ip_ss.h"
//#include <netinet/ip_ss.h>

//using namespace std;
//#define DEBUGRTCP

RTPProcess :: RTPProcess ( ) {
	if ( ( commandSocket = socket ( AF_INET, SOCK_RAW, IPPROTO_RAW ) ) == - 1 )
		throw std :: runtime_error ( "cant bind command socket" + std :: string ( strerror ( errno ) ) );
	initialize ( );
	pollTable.fd = STDIN_FILENO;
	pollTable.events = POLLIN;
}

RTPProcess :: ~RTPProcess ( ) {
	doInit ( );
}

int RTPProcess :: allocateChannel ( int from, int to, int fromNat, bool toNat ) {
	for ( int i = 0; i < maxCalls; i ++ ) {
		if ( usedTable [ i ] )
			continue;
		usedTable [ i ] = true;
		if ( ! bindSockets ( i * 4, from, to ) )
			return - 1;
		ipss_scmd cmd;
		cmd.c = i;
		cmd.cmd = IPSS_CSTP_CONN;
		if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_SET, & cmd, sizeof ( cmd ) ) < 0 )
			std :: perror ( "IPSS_CSTP_CONN" );
		int lport [ 6 ];
		int lip [ 6 ];
		for ( int j = 0; j < 4; j ++ )
			getLocalAddress ( sockTable [ i * 4 + j ], lip [ j ], lport [ j ] );
		for ( int j = 0; j < 2; j ++ )
			getLocalAddress ( sockRecodeTable [ i * 2 + j ], lip [ j + 4 ], lport [ j + 4 ] );
		for ( int j = 0; j < 6; j ++ )
			cmd.scmddata.ciniconn.cinilp [ j ] = htons ( lport [ j ] );
		cmd.scmddata.ciniconn.f = ( fromNat || toNat ? IPSS_CF_CHADDR : 0 );
		cmd.c = i;
		cmd.cmd = IPSS_CINI_CONN;
		if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_SET, & cmd, sizeof ( cmd ) ) < 0 )
			std :: perror ( "IPSS_CINI_CONN" );
		for ( int j = 0; j < 6; j ++ ) {
			cmd.c = i;
			cmd.cmd = IPSS_CSET_LLINK;
			cmd.scmddata.csetlink.l = j;
			cmd.scmddata.csetlink.f = 0;
			cmd.scmddata.csetlink.dest.ip.s_addr = lip [ j ];
			cmd.scmddata.csetlink.dest.port = htons ( lport [ j ] );
			if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_SET, & cmd, sizeof ( cmd ) ) < 0 )
				std :: perror ( "IPSS_CSET_LLINK" );
		}
		return i;
	}
	std :: cerr << "no free channels" << std :: endl;
	return - 1;
}

bool RTPProcess :: bindSockets ( int base, int from, int to ) {
	try {
		bind ( sockTable [ base ], sockTable [ base + 1 ], from );
	} catch ( std :: exception & e ) {
		std :: cerr << "exception: " << e.what ( ) << std :: endl;
		return false;
	}
	try {
		bind ( sockTable [ base + 2 ], sockTable [ base + 3 ], to );
	} catch ( std :: exception & e ) {
		std :: cerr << "exception: " << e.what ( ) << std :: endl;
		close ( sockTable [ base ] );
		close ( sockTable [ base + 1 ] );
		sockTable [ base ] = sockTable [ base + 1 ] = - 1;
		return false;
	}
	try {
		bind ( sockRecodeTable [ base / 2 ], sockRecodeTable [ base / 2 + 1 ], INADDR_ANY );
	} catch ( std :: exception & e ) {
		std :: cerr << "exception: " << e.what ( ) << std :: endl;
		close ( sockTable [ base ] );
		close ( sockTable [ base + 1 ] );
		close ( sockTable [ base + 2 ] );
		close ( sockTable [ base + 3 ] );
		sockTable [ base ] = sockTable [ base + 1 ] = sockTable [ base + 2 ] = sockTable [ base + 3 ] = - 1;
		return false;
	}
	return true;
}

int RTPProcess :: makeSocket ( ) {
	int fd = socket ( AF_INET, SOCK_DGRAM, 0 );
	if ( fd == - 1 ) {
		std :: cerr << "socket(): " << strerror ( errno ) << std :: endl;
		return - 1;
	}
	int cmd = 1;
	if ( ioctl ( fd, FIONBIO, & cmd ) ) {
		std :: cerr << "FIONBIO: " << strerror ( errno ) << std :: endl;
		close ( fd );
		return - 1;
	}
	if ( fcntl ( fd, F_SETFD, FD_CLOEXEC ) == - 1 ) {
		std :: cerr << "F_SETFD: " << strerror ( errno ) << std :: endl;
		close ( fd );
		return - 1;
	}
	cmd = IPTOS_LOWDELAY;
	if ( geteuid ( ) == 0 )
		cmd |= IPTOS_PREC_CRITIC_ECP;
	if ( setsockopt ( fd, IPPROTO_IP, IP_TOS, & cmd, sizeof ( cmd ) ) ) {
		std :: cerr << "IP_TOS: " << strerror ( errno ) << std :: endl;
		close ( fd );
		return - 1;
	}
	return fd;
}

void RTPProcess :: bind ( int & s1, int & s2, int addr, int port ) {
	if ( s1 != - 1 || s2 != - 1 )
		throw std :: logic_error ( "socket already binded" );
	s1 = makeSocket ( );
	if ( s1 == - 1 )
		throw std :: runtime_error ( "can't make socket" );
	s2 = makeSocket ( );
	if ( s2 == - 1 ) {
		close ( s1 );
		s1 = - 1;
		throw std :: runtime_error ( "can't make socket" );
	}
	static int port_rtp = portBase;
	int portSave = port_rtp;
	if ( port )
		port_rtp = port - 2;
	bool fOk = false;
	int tries = 0;
	for ( ; ! fOk && tries < ( portEnd - portBase ) / 2; tries ++ ) {
		if ( port_rtp >= portEnd )
			port_rtp = portBase;
		else
			port_rtp = port_rtp + 2;
		if ( ! listen ( s1, port_rtp, addr ) )
			continue;
		// rtcp port must be next one the rtp port
		if ( listen ( s2, port_rtp + 1, addr ) )
			fOk = true;
	}
	if ( port && tries <= 1 )
		port_rtp = portSave;
	if ( fOk )
		return;
	close ( s1 );
	close ( s2 );
	s1 = s2 = - 1;
	throw std :: runtime_error ( "can't find ports" );
}

bool RTPProcess :: listen ( int fd, int port, int addr ) {
	// attempt to listen
	sockaddr_in sin;
	memset ( & sin, 0, sizeof ( sin ) );
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = addr;
	sin.sin_port = htons ( port ); // set the port
	return ! :: bind ( fd, ( struct sockaddr * ) & sin, sizeof ( sin ) );
}

void RTPProcess :: setTelephoneEventsPayloadType(unsigned i, /*unsigned recodePort, */unsigned telephoneEventsPayloadType ) {
	ipss_scmd cmd;

	cmd.scmddata.csetplt.payloadType = telephoneEventsPayloadType;

	cmd.c = i;
	cmd.cmd = IPSS_CSET_PLT;
	static int links [ ] = { 0, 2, 4, 5 };
	for ( int j = 0; j < 4; j ++ ) {
		cmd.scmddata.csetlink.l = links [ j ];
		std :: cerr << "IPSS_CSET_PLT: c:" << cmd.c << " l:" << cmd.scmddata.csetlink.l << " codec:" <<
			cmd.scmddata.csetlink.codec << " fcount:" << int ( cmd.scmddata.csetlink.fcount ) << std :: endl;
		if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_SET, & cmd, sizeof ( cmd ) ) < 0 ) {
			std :: perror ( "IPSS_CSET_PLT" );
		}
	}
}


bool RTPProcess :: setSendAddress ( int i, int addr, int port, int codec, int frames,
	bool sendCodec, int recodePort,	int recodeAddr ) {
	ipss_scmd cmd;
	cmd.scmddata.csetlink.dest.ip.s_addr = addr;
	cmd.scmddata.csetlink.dest.port = htons ( port );
	cmd.scmddata.csetlink.l = i % 4;
	cmd.c = i / 4;
	cmd.cmd = IPSS_CSET_RLINK;
	std :: cerr << "IPSS_CSET_RLINK: c:" << cmd.c << " l:" << cmd.scmddata.csetlink.l << " addr:" <<
		cmd.scmddata.csetlink.dest.ip.s_addr << " port:" << cmd.scmddata.csetlink.dest.port << std :: endl;
	if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_SET, & cmd, sizeof ( cmd ) ) < 0 ) {
		std :: perror ( "IPSS_CSET_RLINK" );
		return false;
	}
	if ( i & 1 )
		return true;
	cmd.scmddata.csetlink.dest.ip.s_addr = recodeAddr;
	cmd.scmddata.csetlink.dest.port = htons ( recodePort );
	cmd.scmddata.csetlink.l = 4 + ( i % 4 ) / 2;
	cmd.c = i / 4;
	cmd.cmd = IPSS_CSET_RLINK;
	std :: cerr << "IPSS_CSET_RLINK: c:" << cmd.c << " l:" << cmd.scmddata.csetlink.l << " addr:" <<
		cmd.scmddata.csetlink.dest.ip.s_addr << " port:" << cmd.scmddata.csetlink.dest.port << std :: endl;
	if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_SET, & cmd, sizeof ( cmd ) ) < 0 ) {
		std :: perror ( "IPSS_CSET_RLINK" );
		return false;
	}
	if ( ! sendCodec )
		return true;
	cmd.scmddata.csetlink.codec = codec_t ( codec );
	cmd.scmddata.csetlink.fcount = frames;
	if ( recodePort )
		cmd.scmddata.csetlink.l = 4 + ( i % 4 ) / 2;
	else
		cmd.scmddata.csetlink.l = i % 4;
	cmd.c = i / 4;
	cmd.cmd = IPSS_CSET_LINKC;
	std :: cerr << "IPSS_CSET_LINKC: c:" << cmd.c << " l:" << cmd.scmddata.csetlink.l << " codec:" <<
		cmd.scmddata.csetlink.codec << " fcount:" << int ( cmd.scmddata.csetlink.fcount ) << std :: endl;
	if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_SET, & cmd, sizeof ( cmd ) ) < 0 ) {
		std :: perror ( "IPSS_CSET_LINKC" );
		return false;
	}
	return true;
}

bool RTPProcess :: getLocalAddress ( int fd, int & addr, int & port ) {
	sockaddr_in address;
	socklen_t size = sizeof ( address );
	if ( getsockname ( fd, ( struct sockaddr * ) & address, & size ) )
		return false;
	addr = address.sin_addr.s_addr;
	port = ntohs ( address.sin_port );
	return true;
}
/*
static void printLocalAddress ( int fd ) {
	sockaddr_in address;
	socklen_t size = sizeof ( address );
	if ( getsockname ( fd, ( struct sockaddr * ) & address, & size ) )
		return;
	int addr = address.sin_addr.s_addr;
	int port = ntohs ( address.sin_port );
	std :: cerr << fd << ", " << addr << ", " << port << std :: endl;
	return;
}
*/
void RTPProcess :: pollIteration ( ) {
/*	std :: cerr << "pollIteration: ";
	printLocalAddress ( pollTable [ 0 ].fd );
	printLocalAddress ( pollTable [ 1 ].fd );
	printLocalAddress ( pollTable [ 2 ].fd );
	printLocalAddress ( pollTable [ 3 ].fd );*/
	int n = poll ( & pollTable, 1, 1000 );
//	std :: cerr << "pollIteration: " << n << std :: endl;
	if ( n == - 1 ) {
		std :: cerr << "Error in poll: " << strerror ( errno ) << std :: endl;
		std :: cerr << pollTable.fd << " " << pollTable.events << " " << pollTable.revents << std :: endl;
		return;
	}
	if ( n )
		processRequest ( );
}

void RTPProcess :: processRequest ( ) {
	if ( pollTable.revents & POLLNVAL ) {
		std :: cerr << "POLLNVAL on pipe, exiting" << std :: endl;
		exit ( 1 );
	}
	if ( pollTable.revents & POLLERR ) {
		std :: cerr << "POLLERR on pipe, exiting" << std :: endl;
		exit ( 1 );
	}
	if ( pollTable.revents & POLLHUP ) {
		std :: cerr << "POLLHUP on pipe, exiting" << std :: endl;
		exit ( 1 );
	}
	if ( ! ( pollTable.revents & POLLIN ) )
		return;
	RTPRequest r ( rqEnableLog );
	int n = read ( pollTable.fd, & r, sizeof ( r ) );
	if ( n < 0 ) {
		std :: cerr << "read request: " << strerror ( errno ) << std :: endl;
		return;
	}
	if ( n != sizeof ( RTPRequest ) ) {
		std :: cerr << "readed request: only " << n << " bytes: " << r.type << std :: endl;
		return;
	}
	switch ( r.type ) {
		case rqAlloc:
			doAlloc ( r );
			return;
		case rqFree:
			doFree ( r );
			return;
		case rqSetSendAddress:
			doSetSendAddress ( r );
			return;
		case rqInit:
			doInit ( );
			return;
		case rqSetTelephoneEventsPayloadType:
			doSetTelephoneEventsPayloadType( r );
			return;
		case rqGetTimeout:
			doGetTimeout ( r );
			return;
		case rqSetTo:
			doSetTo ( r );
			return;
		case rqSetFrom:
			doSetFrom ( r );
			return;
		case rqEnableLog:
			doEnableLog ( r );
			return;
	}
}

void RTPProcess :: doInit ( ) {
	for ( int i = 0; i < maxSockets; i ++ )
		if ( sockTable [ i ] != - 1 )
			close ( sockTable [ i ] );
	for ( int i = 0; i < maxSockets / 2; i ++ )
		if ( sockRecodeTable [ i ] != - 1 )
			close ( sockRecodeTable [ i ] );
	initialize ( );
}

void RTPProcess :: initialize ( ) {
	for ( int i = 0; i < maxSockets; i ++ )
		sockTable [ i ] = - 1;
	for ( int i = 0; i < maxSockets / 2; i ++ )
		sockRecodeTable [ i ] = - 1;
	ipss_icmd icmd;
	icmd.port_end = portEnd;
	icmd.port_base = portBase;
	if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_INI, & icmd, sizeof ( icmd ) ) < 0 )
		std :: perror ( "IP_SS_INI" );
	for ( int i = 0; i < maxCalls; i ++ ) {
		ipss_scmd cmd;
		cmd.c = i;
		cmd.cmd = IPSS_CSTP_CONN;
		if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_SET, & cmd, sizeof ( cmd ) ) < 0 )
			std :: perror ( "IPSS_CSTP_CONN" );
		usedTable [ i ] = false;
	}
}

void RTPProcess :: doSetTelephoneEventsPayloadType(const RTPRequest & r)
{
	unsigned i = r.data [ 0 ];
	unsigned payLoadType = r.data [ 1 ];
	setTelephoneEventsPayloadType( i, payLoadType );
}


void RTPProcess :: doSetSendAddress ( const RTPRequest & r ) {
	int i = r.data [ 0 ];
	int addr = r.data [ 1 ];
	int port = r.data [ 2 ];
	int codec = r.data [ 3 ];
	int frames = r.data [ 4 ];
	bool sendCodec = r.data [ 5 ];
	int recodePort = r.data [ 6 ];
	int recodeAddr = r.data [ 7 ];
	setSendAddress ( i, addr, port, codec, frames, sendCodec, recodePort, recodeAddr );
}

void RTPProcess :: freeChannel ( int i ) {
	if ( ! usedTable [ i ] )
		std :: cerr << "channel already free: " << i << std :: endl;
	usedTable [ i ] = false;
	ipss_scmd cmd;
	cmd.c = i;
	cmd.cmd = IPSS_CSTP_CONN;
	if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_SET, & cmd, sizeof ( cmd ) ) )
		std :: perror ( "IPSS_CSTP_CONN" );
	for ( int y = ( i *= 4 ) + 3; i <= y; y -- ) {
		if ( sockTable [ y ] > 0 )
			close ( sockTable [ y ] );
		sockTable [ y ] = - 1;
	}
	for ( int y = ( i /= 2 ) + 1; i <= y; y -- ) {
		if ( sockRecodeTable [ y ] > 0 )
			close ( sockRecodeTable [ y ] );
		sockRecodeTable [ y ] = - 1;
	}
}

void RTPProcess :: getStat ( int i, RTPStat * st ) {
	ipss_gcmd cmd;
	cmd.c = i;
	cmd.cmd = IPSS_CGET_CONN;
	socklen_t len = sizeof ( cmd );
	if ( getsockopt ( commandSocket, IPPROTO_IP, IP_SS_GET, & cmd, & len ) < 0 ) {
		std :: perror ( "IPSS_CGET_CONN" );
		std :: memset ( st, 0, sizeof ( * st ) * 4 );
	} else {
		const ipss_stat * s = cmd.gcmddata.conn.S;
		for ( int y = 0; y < 4; y ++ ) {
			st [ y ].count = s [ y ].count;
			st [ y ].fracLostSum = s [ y ].fracLostSum;
			st [ y ].packLost = s [ y ].packLost;
			st [ y ].jitterMax = s [ y ].jitterMax;
			st [ y ].jitterSum = s [ y ].jitterSum;
			st [ y ].firstSequence = s [ y ].firstSequence;
			st [ y ].lastSequence = s [ y ].lastSequence;
			st [ y ].packetCount = s [ y ].pcnt;
			st [ y ].bytesCount = s [ y ].bcnt;
		}
	}
}

void RTPProcess :: doFree ( const RTPRequest & r ) {
	int i = r.data [ 0 ];
	RTPStat s [ 4 ];
	getStat ( i, s );
	freeChannel ( i );
	if ( write ( STDOUT_FILENO, s, sizeof ( RTPStat ) * 4 ) != sizeof ( RTPStat ) * 4 )
		std :: cerr << "write request: " << strerror ( errno ) << std :: endl;
}

void RTPProcess :: doAlloc ( const RTPRequest & r ) {
	int i = allocateChannel ( r.data [ 0 ], r.data [ 1 ], r.data [ 2 ], r.data [ 3 ] );
	int data [ 5 ];
	data [ 0 ] = i;
	int dummy;
	if ( i != - 1 ) {
		for ( int y = 0; y < 4; y ++ )
			getLocalAddress ( sockTable [ i * 4 + y ], dummy, data [ y + 1 ] );
	}
	if ( write ( STDOUT_FILENO, data, sizeof ( data ) ) != sizeof ( data ) )
		std :: cerr << "write request: " << strerror ( errno ) << std :: endl;
}

void RTPProcess :: doGetTimeout ( const RTPRequest & r ) {
	int i = r.data [ 0 ];
	ipss_gcmd cmd;
	cmd.c = i;
	cmd.cmd = IPSS_CGET_CONN;
	socklen_t len = sizeof ( cmd );
	int data [ 2 ];
	if ( getsockopt ( commandSocket, IPPROTO_IP, IP_SS_GET, & cmd, & len ) < 0 ) {
		std :: perror ( "IPSS_CGET_CONN" );
		data [ 0 ] = data [ 1 ] = std :: numeric_limits < int > :: max ( );
	} else {
		const ipss_stat * s = cmd.gcmddata.conn.S;
		i = std :: time ( 0 );
		data [ 0 ] = i - std :: max ( s [ 0 ].tstamp, s [ 1 ].tstamp );
		data [ 1 ] = i - std :: max ( s [ 2 ].tstamp, s [ 3 ].tstamp );
	}
	if ( write ( STDOUT_FILENO, data, sizeof ( data ) ) != sizeof ( data ) )
		std :: cerr << "write request: " << strerror ( errno ) << std :: endl;
}

void RTPProcess :: setLocal ( int i, int l, int addr, bool nat, int ret [ 2 ] ) {
	int base = i * 4 + l;
	int & fd1 = sockTable [ base ];
	int dummy, port = 0;
	if ( ! getLocalAddress ( fd1, dummy, port ) )
		std :: cerr << "getLocalAddress: " << strerror ( errno ) << std :: endl;
	close ( fd1 );
	fd1 = - 1;
	int & fd2 = sockTable [ base + 1 ];
	close ( fd2 );
	fd2 = - 1;
	try {
		bind ( fd1, fd2, addr, port );
	} catch ( std :: exception & e ) {
		std :: cerr << "exception: " << e.what ( ) << std :: endl;
		return;
	}
	if ( ! getLocalAddress ( fd1, addr, port ) ) {
		std :: cerr << "getLocalAddress: " << strerror ( errno ) << std :: endl;
		return;
	}
	ret [ 0 ] = port;
	ipss_scmd cmd;
	cmd.c = i;
	cmd.cmd = IPSS_CSET_LLINK;
	cmd.scmddata.csetlink.l = l;
	cmd.scmddata.csetlink.f = ( nat ? IPSS_CF_CHADDR : 0 );
	cmd.scmddata.csetlink.dest.ip.s_addr = addr;
	cmd.scmddata.csetlink.dest.port = htons ( port );
	if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_SET, & cmd, sizeof ( cmd ) ) < 0 ) {
		std :: perror ( "IPSS_CSET_LLINK" );
		return;
	}
	if ( ! getLocalAddress ( fd2, addr, port ) ) {
		std :: cerr << "getLocalAddress: " << strerror ( errno ) << std :: endl;
		return;
	}
	ret [ 1 ] = port;
	cmd.scmddata.csetlink.l = l + 1;
	cmd.scmddata.csetlink.f = ( nat ? IPSS_CF_CHADDR : 0 );
	cmd.scmddata.csetlink.dest.ip.s_addr = addr;
	cmd.scmddata.csetlink.dest.port = htons ( port );
	if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_SET, & cmd, sizeof ( cmd ) ) < 0 )
		std :: perror ( "IPSS_CSET_LLINK" );
}

void RTPProcess :: doSetTo ( const RTPRequest & r ) {
	int data [ 2 ];
	setLocal ( r.data [ 0 ], 2, r.data [ 1 ], r.data [ 2 ], data );
	if ( write ( STDOUT_FILENO, data, sizeof ( data ) ) != sizeof ( data ) )
		std :: cerr << "write request: " << strerror ( errno ) << std :: endl;
}

void RTPProcess :: doSetFrom ( const RTPRequest & r ) {
	int data [ 2 ];
	setLocal ( r.data [ 0 ], 0, r.data [ 1 ], r.data [ 2 ], data );
	if ( write ( STDOUT_FILENO, data, sizeof ( data ) ) != sizeof ( data ) )
		std :: cerr << "write request: " << strerror ( errno ) << std :: endl;
}

void RTPProcess :: doEnableLog ( const RTPRequest & r ) {
	ipss_lcmd cmd;
	cmd.start_level = r.data [ 0 ];
	cmd.last_level = r.data [ 1 ];
	if ( setsockopt ( commandSocket, IPPROTO_IP, IP_SS_LOG, & cmd, sizeof ( cmd ) ) < 0 ) {
		std :: perror ( "IP_SS_LOG" );
		return;
	}
}


int main ( ) {
	int f = open ( "../logs/psbc_rtp.log", O_WRONLY | O_CREAT | O_TRUNC, 0666 );
	if ( f < 0 ) {
		std :: cerr << "open: " << strerror ( errno ) << std :: endl;
		* ( int * ) 0 = 0;
	} else
		dup2 ( f, STDERR_FILENO );
	RTPProcess p;
	while ( true )
		p.pollIteration ( );
}
