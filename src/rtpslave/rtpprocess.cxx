#include "rtpprocess.hpp"
#include <cstring>
#include "../rtprequest.hpp"
#include <unistd.h>
#include <iostream>
#include <cerrno>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdexcept>
#include <iomanip>
#include <sys/time.h>
#include <arpa/inet.h>
//using namespace std;
//#define DEBUGRTCP
void RTPSource :: printStats ( ) const {
	unsigned extendedMax = cycles + maxSeq;
	unsigned expected = extendedMax - baseSeq + 1;
	int lost = expected - received;
	std :: cerr << "expected: " << expected << ", lost: " << lost << ", " << "jitter: "
		<< jitter / 16 << std :: endl;
}
unsigned RTPSource :: getFracLost ( int count ) const {
	unsigned extendedMax = cycles + maxSeq;
	unsigned expected = extendedMax - baseSeq + 1;
	int lost = expected - received;
	if ( ! expected )
		expected = 1;
	if ( lost < 0 )
		lost = 0;
	return lost * 256 / expected * count;
}
unsigned RTPSource :: getJitter ( ) const {
	return jitter;
}
void RTPSource :: reset ( ) {
	ssrc = 0;
}
const unsigned int RTP_SEQ_MOD = 1 << 16;
const int MIN_SEQUENTIAL = 2;
static unsigned long long currentTimeStamp;
static std :: time_t currentTime;
static std :: string currentTimeStr;
static void makeTimeStr ( ) {
	tm tms;
	gmtime_r ( & currentTime, & tms );
	char s [ 40 ] = "";
	strftime ( s, sizeof ( s ), "%Y/%m/%d %H:%M:%S", & tms );
	currentTimeStr = s;
}
void RTPSource :: initFirst ( unsigned short seq, unsigned ts, unsigned s ) {
	ssrc = s;
	probation = MIN_SEQUENTIAL;
	maxSeq = seq - 1;
	transit = currentTimeStamp - ts;
	jitter = 0;
}
void RTPSource :: init ( unsigned short seq ) {
	baseSeq = seq;
	maxSeq = seq;
	badSeq = RTP_SEQ_MOD + 1;
	cycles = 0;
	received = 0;
	receivedPrior = 0;
	expectedPrior = 0;
}
static long long getTimeStamp ( ) {
	timeval tv;
	gettimeofday ( & tv, 0 );
	long long t = tv.tv_sec;
	t *= 8000;
	t += tv.tv_usec / 125;
	return t;
}

bool RTPSource :: update ( unsigned short seq, unsigned ts, unsigned s ) {
	if ( s != ssrc )
		initFirst ( seq, ts, s );
	const int MAX_DROPOUT = 3000;
	const int MAX_MISORDER = 100;
	if ( probation ) {
		if ( seq == maxSeq + 1 ) {
			maxSeq = seq;
			if ( -- probation == 0 ) {
				init ( seq );
				received ++;
				return true;
			}
		} else {
			probation = MIN_SEQUENTIAL - 1;
			maxSeq = seq;
		}
		return false;
	}
	unsigned short udelta = seq - maxSeq;
	if ( udelta < MAX_DROPOUT ) {
		// in order, with permissible gap
		if ( seq < maxSeq )
			//Sequence number wrapped - count another 64K cycle
			cycles += RTP_SEQ_MOD;
		maxSeq = seq;
	} else if ( udelta <= RTP_SEQ_MOD - MAX_MISORDER ) {
		// the sequence number made a very large jump
		if ( seq == badSeq )
			// Two sequential packets -- assume that the other side
			// restarted without telling us so just re-sync
			// (i.e., pretend this was the first packet).
			init ( seq );
		else {
			badSeq = ( seq + 1 ) & ( RTP_SEQ_MOD - 1 );
			return false;
		}
	} else {
		// duplicate or reordered packet
	}
	received ++;
	long long trans = currentTimeStamp - ts;
	int d = trans - transit;
	if ( d < 0 )
		d = - d;
	jitter += d - ( jitter + 8 >> 4 );
#ifdef DEBUGRTCP
	std :: cerr << "currentTimeStamp: " << currentTimeStamp << ", s: " << s <<
		", seq: " << seq << ", ts: " << ts << ", trans: " << trans <<
		", transit: " << transit << ", d: " << d << ", jitter: " <<
		jitter << std :: endl;
#endif
	transit = trans;
	return true;
}


RTPProcess :: RTPProcess ( ) {
	initialize ( );
	pollTable [ maxSockets ].fd = STDIN_FILENO;
	pollTable [ maxSockets ].events = POLLIN;
}
RTPProcess :: ~RTPProcess ( ) {
	doInit ( );
}
int RTPProcess :: allocateChannel ( int from, int to ) {
	for ( int i = 0; i < maxCalls; i ++ )
		if ( ! usedTable [ i ] ) {
			usedTable [ i ] = true;
			lastActiveTimes [ i * 2 ] = lastActiveTimes [ i * 2 + 1 ] = currentTime;
			if ( bindSockets ( i * 4, from, to ) )
				return i;
			usedTable [ i ] = false;
			std :: cerr << "can't bind sockets" << std :: endl;
			return - 1;
		}
	std :: cerr << "no free channels" << std :: endl;
	return - 1;
}
bool RTPProcess :: bindSockets ( int base, int from, int to ) {
	try {
		bind ( pollTable [ base ].fd, pollTable [ base + 1 ].fd, from );
	} catch ( std :: exception & e ) {
		std :: cerr << "exception: " << e.what ( ) << std :: endl;
		return false;
	}
	try {
		bind ( pollTable [ base + 2 ].fd, pollTable [ base + 3 ].fd, to );
	} catch ( std :: exception & e ) {
		std :: cerr << "exception: " << e.what ( ) << std :: endl;
		close ( pollTable [ base ].fd );
		close ( pollTable [ base + 1 ].fd );
		pollTable [ base ].fd = pollTable [ base + 1 ].fd = - 1;
		return false;
	}
	return true;
}
int RTPProcess :: makeSocket ( ) {
	int fd = socket ( AF_INET, SOCK_DGRAM, 0 );
	if ( fd == - 1 ) {
		std :: cerr << "socket(): " << std :: strerror ( errno ) << std :: endl;
		return - 1;
	}
	int cmd = 1;
	if ( ioctl ( fd, FIONBIO, & cmd ) ) {
		std :: cerr << "FIONBIO: " << std :: strerror ( errno ) << std :: endl;
		close ( fd );
		return - 1;
	}
	if ( fcntl ( fd, F_SETFD, FD_CLOEXEC ) == - 1 ) {
		std :: cerr << "F_SETFD: " << std :: strerror ( errno ) << std :: endl;
		close ( fd );
		return - 1;
	}
	cmd = IPTOS_LOWDELAY;
	if ( geteuid ( ) == 0 )
		cmd |= IPTOS_PREC_CRITIC_ECP;
	if ( setsockopt ( fd, IPPROTO_IP, IP_TOS, & cmd, sizeof ( cmd ) ) ) {
		std :: cerr << "IP_TOS: " << std :: strerror ( errno ) << std :: endl;
		close ( fd );
		return - 1;
	}
/*	cmd = 4096;
	if ( setsockopt ( fd, SOL_SOCKET, SO_RCVBUF, & cmd, sizeof ( cmd ) ) ) {
		std :: cerr << "SO_RCVBUF: " << std :: strerror ( errno ) << std :: endl;
		close ( fd );
		return - 1;
	}
	if ( setsockopt ( fd, SOL_SOCKET, SO_SNDBUF, & cmd, sizeof ( cmd ) ) ) {
		std :: cerr << "SO_SNDBUF: " << std :: strerror ( errno ) << std :: endl;
		close ( fd );
		return - 1;
	}*/
/*	cmd = 0;
	if ( setsockopt ( fd, SOL_SOCKET, SO_REUSEADDR, & cmd, sizeof ( cmd ) ) ); {
		std :: cerr << "SO_REUSEADDR: " << std :: strerror ( errno ) << std :: endl;
		close ( fd );
		return - 1;
	}*/
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
	std :: memset ( & sin, 0, sizeof ( sin ) );
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = addr;
	sin.sin_port = htons ( port ); // set the port
	return ! :: bind ( fd, ( struct sockaddr * ) & sin, sizeof ( sin ) );
}
bool RTPProcess :: setSendAddress ( int i, int addr, int port ) {
	sockaddr_in & sin = destAddrs [ i ];
	std :: memset ( & sin, 0, sizeof ( sin ) );
	sin.sin_family = AF_INET;
	sin.sin_port = htons ( port ); // set the port
	sin.sin_addr.s_addr = addr;
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
void RTPProcess :: pollIteration ( ) {
/*	std :: cerr << "pollIteration: ";
	printLocalAddress ( pollTable [ 0 ].fd );
	printLocalAddress ( pollTable [ 1 ].fd );
	printLocalAddress ( pollTable [ 2 ].fd );
	printLocalAddress ( pollTable [ 3 ].fd );*/
	int n = poll ( pollTable, maxSockets + 1, 1000 );
//	std :: cerr << "pollIteration: " << n << endl;
	if ( n == - 1 ) {
		std :: cerr << "Error in poll: " << std :: strerror ( errno ) << std :: endl;
		for ( int i = 0; i < 5; i ++ )
			std :: cerr << pollTable [ i ].fd << " " << pollTable [ i ].events << " " <<
				pollTable [ i ].revents << std :: endl;
		std :: cerr << pollTable [ maxSockets ].fd << " " << pollTable [ maxSockets ].events << " " <<
			pollTable [ maxSockets ].revents << std :: endl;
		return;
	}
	if ( pollTable [ maxSockets ].revents )
		n --;
	currentTimeStamp = getTimeStamp ( );
	currentTime = std :: time ( 0 );
	for ( int i = 0; n > 0 && i < maxSockets; i ++ ) {
		if ( ! pollTable [ i ].revents )
			continue;
		n --;
		if ( pollTable [ i ].revents & POLLIN ) {
			doProxy ( i );
			continue;
		}
		if ( pollTable [ i ].revents & POLLNVAL ) {
			close ( pollTable [ i ].fd );
			pollTable [ i ].fd = - 1;
			std :: cerr << "POLLNVAL on channel " << i << std :: endl;
			continue;
		}
		if ( pollTable [ i ].revents & POLLERR ) {
			close ( pollTable [ i ].fd );
			pollTable [ i ].fd = - 1;
			std :: cerr << "POLLERR on channel " << i << std :: endl;
			continue;
		}
		if ( pollTable [ i ].revents & POLLHUP ) {
			close ( pollTable [ i ].fd );
			pollTable [ i ].fd = - 1;
			std :: cerr << "POLLHUP on channel " << i << std :: endl;
		}
	}
	processRequest ( );
}
void RTPProcess :: processRequest ( ) {
	if ( pollTable [ maxSockets ].revents & POLLNVAL ) {
		std :: cerr << "POLLNVAL on pipe, exiting" << std :: endl;
		exit ( 1 );
	}
	if ( pollTable [ maxSockets ].revents & POLLERR ) {
		std :: cerr << "POLLERR on pipe, exiting" << std :: endl;
		exit ( 1 );
	}
	if ( pollTable [ maxSockets ].revents & POLLHUP ) {
		std :: cerr << "POLLHUP on pipe, exiting" << std :: endl;
		exit ( 1 );
	}
	if ( ! ( pollTable [ maxSockets ].revents & POLLIN ) )
		return;
	RTPRequest r ( rqAlloc );
	int n = read ( pollTable [ maxSockets ].fd, & r, sizeof ( r ) );
	if ( n < 0 ) {
		std :: cerr << "read request: " << std :: strerror ( errno ) << std :: endl;
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
		case rqGetTimeout:
			doGetTimeout ( r );
			return;
		case rqEnableLog:
			doEnableLog ( r );
			return;
		case rqSetTo:
			doSetTo ( r );
			return;
		case rqSetFrom:
			doSetFrom ( r );
			return;
	}
}
void RTPProcess :: doInit ( ) {
	for ( int i = 0; i < maxSockets; i ++ )
		if ( pollTable [ i ].fd != - 1 )
			close ( pollTable [ i ].fd );
	initialize ( );
}
void RTPProcess :: initialize ( ) {
	for ( int i = 0; i < maxSockets; i ++ ) {
		pollTable [ i ].fd = - 1;
		pollTable [ i ].events = POLLIN;
		usedTable [ i ] = false;
	}
}
void RTPProcess :: doSetSendAddress ( const RTPRequest & r ) {
	int i = r.data [ 0 ];
	int addr = r.data [ 1 ];
	int port = r.data [ 2 ];
	setSendAddress ( i, addr, port );
}
void RTPProcess :: freeChannel ( int i ) {
	if ( ! usedTable [ i ] )
		std :: cerr << "channel already free: " << i << std :: endl;
	usedTable [ i ] = false;
	for ( int y = ( i *= 4 ) + 4; i < y; i ++ ) {
		if ( pollTable [ i ].fd )
			close ( pollTable [ i ].fd );
		pollTable [ i ].fd = - 1;
	}
}
void RTPProcess :: doFree ( const RTPRequest & r ) {
	int i = r.data [ 0 ];
	freeChannel ( i );
#ifdef DEBUGRTCP
	rtpSources [ i * 2 ].printStats ( );
	rtpSources [ i * 2 + 1 ].printStats ( );
#endif
	RTPStat * s = statTable + i * 4;
	RTPSource * so = rtpSources + i * 2;
	s -> fracLostSum = so -> getFracLost ( s -> count );
	s += 2;
	so ++;
	s -> fracLostSum = so -> getFracLost ( s -> count );
	if ( write ( STDOUT_FILENO, statTable + i * 4, sizeof ( * statTable ) * 4 )
		!= sizeof ( * statTable ) * 4 )
		std :: cerr << "write request: " << std :: strerror ( errno ) << std :: endl;
}
void RTPProcess :: doAlloc ( const RTPRequest & r ) {
	int i = allocateChannel ( r.data [ 0 ], r.data [ 1 ] );
	int data [ 5 ];
	data [ 0 ] = i;
	int dummy;
	if ( i != - 1 ) {
		for ( int y = 0; y < 4; y ++ )
			getLocalAddress ( pollTable [ i * 4 + y ].fd, dummy, data [ y + 1 ] );
		statTable [ i * 4 ] = statTable [ i * 4 + 1 ] =
			statTable [ i * 4 + 2 ] = statTable [ i * 4 + 3 ] =
			RTPStat ( );
		rtpSources [ i * 2 ].reset ( );
		rtpSources [ i * 2 + 1 ].reset ( );
		log [ i ] = false;
		fromNat [ i ] = r.data [ 2 ];
		toNat [ i ] = r.data [ 3 ];
	}
	if ( write ( STDOUT_FILENO, data, sizeof ( data ) ) != sizeof ( data ) )
		std :: cerr << "write request: " << std :: strerror ( errno ) << std :: endl;
}
void RTPProcess :: doGetTimeout ( const RTPRequest & r ) {
	int data [ 2 ];
	data [ 0 ] = currentTime - lastActiveTimes [ r.data [ 0 ] * 2 ];
	data [ 1 ] = currentTime - lastActiveTimes [ r.data [ 0 ] * 2 + 1 ];
	if ( write ( STDOUT_FILENO, & data, sizeof ( data ) ) != sizeof ( data ) )
		std :: cerr << "write request: " << std :: strerror ( errno ) << std :: endl;
}
void RTPProcess :: doEnableLog ( const RTPRequest & r ) {
	log [ r.data [ 0 ] ] = true;
}
void RTPProcess :: doSetTo ( const RTPRequest & r ) {
	toNat [ r.data [ 0 ] ] = r.data [ 2 ];
	int & fd1 = pollTable [ r.data [ 0 ] * 4 + 2 ].fd;
	int dummy, port = 0;
	getLocalAddress ( fd1, dummy, port );
	close ( fd1 );
	fd1 = - 1;
	int & fd2 = pollTable [ r.data [ 0 ] * 4 + 3 ].fd;
	close ( fd2 );
	fd2 = - 1;
	int addr = r.data [ 1 ];
	try {
		bind ( fd1, fd2, addr, port );
	} catch ( std :: exception & e ) {
		std :: cerr << "exception: " << e.what ( ) << std :: endl;
	}
	int data [ 2 ];
	getLocalAddress ( fd1, dummy, data [ 0 ] );
	getLocalAddress ( fd2, dummy, data [ 1 ] );
	if ( write ( STDOUT_FILENO, & data, sizeof ( data ) ) != sizeof ( data ) )
		std :: cerr << "write request: " << std :: strerror ( errno ) << std :: endl;
}
void RTPProcess :: doSetFrom ( const RTPRequest & r ) {
	toNat [ r.data [ 0 ] ] = r.data [ 2 ];
	int & fd1 = pollTable [ r.data [ 0 ] * 4 ].fd;
	close ( fd1 );
	fd1 = - 1;
	int & fd2 = pollTable [ r.data [ 0 ] * 4 + 1 ].fd;
	close ( fd2 );
	fd2 = - 1;
	int addr = r.data [ 1 ];
	try {
		bind ( fd1, fd2, addr );
	} catch ( std :: exception & e ) {
		std :: cerr << "exception: " << e.what ( ) << std :: endl;
	}
	int data [ 2 ], dummy;
	getLocalAddress ( fd1, dummy, data [ 0 ] );
	getLocalAddress ( fd2, dummy, data [ 1 ] );
	if ( write ( STDOUT_FILENO, & data, sizeof ( data ) ) != sizeof ( data ) )
		std :: cerr << "write request: " << std :: strerror ( errno ) << std :: endl;
}
static void printPeerName ( int fd ) {
	sockaddr_in address;
	socklen_t size = sizeof ( address );
	if ( getpeername ( fd, ( struct sockaddr * ) & address, & size ) ) {
		std :: cerr << "getpeername: " << std :: strerror ( errno );
		return;
	}
	int addr = address.sin_addr.s_addr;
	int port = ntohs ( address.sin_port );
	std :: cerr << "ip: " << addr << ", port: " << port;
}
static void printBuffer ( const unsigned char * buf, int len ) {
	std :: cerr << "packet:\n" << std :: hex;
	for ( int i = 0; i < len; i ++ ) {
		std :: cerr << std :: setprecision ( 2 ) << ( int ) buf [ i ] << ' ';
		if ( ( i & 15 ) == 15 )
			std :: cerr << std :: endl;
	}
	if ( len & 15 )
		std :: cerr << std :: endl;
	std :: cerr << std :: dec;
}
static unsigned charToInt ( const unsigned char * buf ) {
	return buf [ 0 ] * 65536U * 256 + buf [ 1 ] * 65536U + buf [ 2 ] * 256U + buf [ 3 ];
}
struct RepData {
	unsigned ssrc;
	unsigned fracLost;
	unsigned packLost;
	unsigned seqReceived;
	unsigned jitter;
	unsigned lsr;
	unsigned dlsr;
};
const int repSize = 24;
static const unsigned char * getRepPtr ( const unsigned char * buf, int len,
	int & count, unsigned & ssrc ) {
	count = 0;
	if ( len < 4 ) {
#ifdef DEBUGRTCP
		std :: cerr << "len: " << len << std :: endl;
#endif
		return 0;
	}
	if ( ( * buf & 192 ) != 128 ) {
#ifdef DEBUGRTCP
		std :: cerr << "isn't second version" << std :: endl;
#endif
		return 0;
	}
	int repOff = 0;
	if ( buf [ 1 ] == 200 )
		repOff = 28;
	else if ( buf [ 1 ] == 201 )
		repOff = 8;
	else {
#ifdef DEBUGRTCP
		std :: cerr << "isn't report" << std :: endl;
#endif
		return 0;
	}
	int l = buf [ 2 ] * 256 + buf [ 3 ];
	if ( ( l + 1 ) * 4 > len ) {
#ifdef DEBUGRTCP
		std :: cerr << "packet len " << ( l + 1 ) * 4 << " greater than " << len << std :: endl;
#endif
		return 0;
	}
	int cnt = buf [ 0 ] & 31;
	if ( ! cnt ) {
#ifdef DEBUGRTCP
		std :: cerr << "report count is zero" << std :: endl;
#endif
		return 0;
	}
	if ( ( l + 1 ) * 4 < repOff + cnt * repSize ) {
#ifdef DEBUGRTCP
		std :: cerr << "reports " << repOff + cnt * repSize <<
			" dont fit in paket " << ( l + 1 ) * 4 << std :: endl;
#endif
		return 0;
	}
	count = cnt;
	ssrc = charToInt ( buf + 4 );
	return buf + repOff;
}
static void getRepData ( RepData & rd, const unsigned char * buf ) {
	rd.ssrc = charToInt ( buf );
	rd.fracLost = buf [ 4 ];
	rd.packLost = buf [ 5 ] * 65536 + buf [ 6 ] * 256 +
		buf [ 7 ];
	rd.seqReceived = charToInt ( buf + 8 );
	rd.jitter = charToInt ( buf + 12 );
	rd.lsr = charToInt ( buf + 16 );
	rd.dlsr = charToInt ( buf + 20 );
}

static void printSR ( const RepData & rd ) {
	std :: cerr << "ssrc: " << rd.ssrc << std :: endl;
	std :: cerr << "fraction lost: " << rd.fracLost << std :: endl;
	std :: cerr << "packets lost: " << rd.packLost << std :: endl;
	std :: cerr << "last sequence received: " << rd.seqReceived << std :: endl;
	std :: cerr << "jitter: " << rd.jitter << std :: endl;
	std :: cerr << "lsr: " << rd.lsr << std :: endl;
	std :: cerr << "dlsr: " << rd.dlsr << std :: endl;
}
void RTPProcess :: getStats ( int i, const RepData & rd, unsigned ssrc ) {
	if ( ssrc == rd.ssrc )
		return;
	RTPStat & s = statTable [ i ];
	s.fracLostSum += rd.fracLost;
	s.packLost = rd.packLost;
	s.lastSequence = rd.seqReceived;
	if ( ! s.count )
		s.firstSequence = rd.seqReceived;
	if ( s.jitterMax < rd.jitter )
		s.jitterMax = rd.jitter;
	s.jitterSum += rd.jitter;
	s.count ++;
}
void RTPProcess :: doProxy ( int i ) {
	const int bufLen = 2000;
	static unsigned char buf [ bufLen ];
	int len;
	sockaddr_in fromAddr;
	socklen_t sl = sizeof ( fromAddr );
	len = recvfrom ( pollTable [ i ].fd, buf, bufLen, 0,
		reinterpret_cast < sockaddr * > ( & fromAddr ), & sl );
	if ( len == -1 ) {
		std :: cerr << "Read: " << std :: strerror ( errno ) << std :: endl;
		return;
	}
	if ( len == 0 ) {
		std :: cerr << "Read: nothing to read" << std :: endl;
		return;
	}
	if ( len == bufLen )
		std :: cerr << "Reading buffer filled" << std :: endl;
#ifdef DEBUGRTCP
	std :: cerr << "packet from ip: " << fromAddr.sin_addr.s_addr << ", port: " <<
		ntohs ( fromAddr.sin_port ) << std :: endl;
#else
	if ( log [ i / 4 ] ) {
		makeTimeStr ( );
		int addr, port;
		char astr [ 40 ];
		getLocalAddress ( pollTable [ i ].fd, addr, port );
		std :: cerr << currentTimeStr << ": " << i / 4 << ": packet from ip: " <<
			inet_ntop ( AF_INET, & fromAddr.sin_addr.s_addr, astr, 40 ) << ", port: " <<
			ntohs ( fromAddr.sin_port ) << ", dport: " << port << std :: endl;
	}
#endif
	if ( i & 1 ) {
#ifdef DEBUGRTCP
		std :: cerr << i << std :: endl;
		printBuffer ( buf, len );
#endif
		int cnt;
		unsigned ssrc;
		const unsigned char * reps = getRepPtr ( buf, len, cnt, ssrc );
#ifdef DEBUGRTCP
		std :: cerr << cnt << " reports" << std :: endl;
#endif
		while ( cnt -- ) {
			if ( ! reps )
				break;
			RepData rd;
			getRepData ( rd, reps );
#ifdef DEBUGRTCP
			printSR ( rd );
#endif
			getStats ( i, rd, ssrc );
			reps += repSize;
		}
		if ( i % 4 == 1 && fromNat [ i / 4 ] && ( statTable [ i ].count == 1 || fromNat [ i / 4 ] == 2 ) )
			destAddrs [ i + 2 ] = fromAddr;
		if ( i % 4 == 3 && toNat [ i / 4 ] )
			destAddrs [ i - 2 ] = fromAddr;
	} else {
		unsigned short seq = buf [ 2 ] * 256 + buf [ 3 ];
		unsigned ts = charToInt ( buf + 4 );
		unsigned s = charToInt ( buf + 8 );
		RTPSource & so = rtpSources [ i / 2 ];
		so.update ( seq, ts, s );
		RTPStat & st = statTable [ i ];
		unsigned jitter = so.getJitter ( ) >> 4;
		if ( st.jitterMax < jitter )
			st.jitterMax = jitter;
		st.jitterSum += jitter;
		st.count ++;
		if ( i % 4 == 0 && fromNat [ i / 4 ] && ( st.count == 1 || fromNat [ i / 4 ] == 2 ) )
			destAddrs [ i + 2 ] = fromAddr;
		if ( i % 4 == 2 && toNat [ i / 4 ] )
			destAddrs [ i - 2 ] = fromAddr;
		lastActiveTimes [ i / 2 ] = currentTime;
	}
	statTable [ i ].packetCount ++;
	statTable [ i ].bytesCount += len;
	int r = sendto ( pollTable [ i ^ 2 ].fd, buf, len, 0,
		reinterpret_cast < sockaddr * > ( & destAddrs [ i ] ),
		sizeof ( destAddrs [ i ] ) );
#ifdef DEBUGRTCP
	std :: cerr << "packet sent to ip: " << destAddrs [ i ].sin_addr.s_addr << ", port: " <<
		ntohs ( destAddrs [ i ].sin_port ) << std :: endl;
#else
	if ( log [ i / 4 ] ) {
		char astr [ 40 ];
		std :: cerr << currentTimeStr << ": " << i / 4 << ": packet sent to ip: " <<
			inet_ntop ( AF_INET, & destAddrs [ i ].sin_addr.s_addr, astr, 40 ) <<
			", port: " << ntohs ( destAddrs [ i ].sin_port ) << std :: endl;
	}
#endif
	if ( r == len )
		return;
	switch ( r ) {
		case - 1:
			std :: cerr << "sendto: " << std :: strerror ( errno ) << " : ";
			printPeerName ( pollTable [ i ].fd );
			std :: cerr << std :: endl;
			return;
		case 0:
			std :: cerr << "sendto: can't write" << std :: endl;
			return;
	}
	std :: cerr << "sendto: can't write " << len << " bytes, only " << r << std :: endl;
}

int main ( ) {
	RTPProcess p;
	int f = open ( "BB.log", O_WRONLY | O_CREAT | O_TRUNC, 0666 );
	if ( f < 0 ) {
		std :: cerr << "open: " << std :: strerror ( errno ) << std :: endl;
		* ( int * ) 0 = 0;
	} else
		dup2 ( f, STDERR_FILENO );
	while ( true )
		p.pollIteration ( );
}
