#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "mgcp.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "mgcpgw.hpp"
#include "pointer.hpp"
#include "mgcpep.hpp"
#include <ptlib/svcproc.h>
#include <boost/regex.hpp>
#include <boost/function_output_iterator.hpp>
#include "setthreeway.hpp"
#include "firstiterator.hpp"
#include "nop.hpp"
#include "istringless.hpp"
#include "mgcpconf.hpp"
#include <boost/mem_fn.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include "transformiterator.hpp"

class Gw :: Impl : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	ss :: string name;
	ss :: string ip;
	typedef std :: map < ss :: string, Pointer < Ep >, istringless < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, Pointer < Ep > > > > EpMap;
	EpMap eps;
	unsigned tid;
	unsigned tries;
	unsigned short port, maxDatagram;
	void checkEndpoints ( const MGCP :: PDU & pdu );
	void sendAuep ( MGCP :: PduVector & ret );
public:
	Impl ( Gw * g, Impl * & i, const ss :: string & n, const ss :: string & a, const MgcpEndpointInfoVector & e,
		unsigned short p, const MgcpClassInfo & cl );
	const ss :: string & getName ( ) const {
		return name;
	}
	const ss :: string & getIp ( ) const {
		return ip;
	}
	unsigned short getPort ( ) const {
		return port;
	}
	unsigned short getMaxDatagram ( ) const {
		return maxDatagram;
	}
	void handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia );
	void init ( MGCP :: PduVector & ret );
	void handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const ss :: string & ep );
	void transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu );
	void detachThreadHandler ( MGCP :: PduVector & ret, const ss :: string & ep, bool ok );
	bool originateCall ( const ss :: string & ep, const OriginateCallArg & a );
	void sendRinging ( MGCP :: PduVector & ret, const ss :: string & ep, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec );
	void sendOk ( MGCP :: PduVector & ret, const ss :: string & ep, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType );
	void reloadConf ( MGCP :: PduVector & ret, const MgcpGatewayInfo & gi, Gw * g );
};

Gw :: Impl :: Impl ( Gw * g, Impl * & i, const ss :: string & n, const ss :: string & a, const MgcpEndpointInfoVector & e,
	unsigned short p, const MgcpClassInfo & cl ) : name ( n ), ip ( a ), tid ( 0 ), tries ( 1 ), port ( p ),
	maxDatagram ( cl.maxDatagram ) {
	i = this;
	for ( MgcpEndpointInfoVector :: const_iterator i = e.begin ( ); i != e.end ( ); ++ i ) {
		Pointer < Ep > p = new Ep ( i -> name, g, i -> inPeer, cl );
		eps.insert ( std :: make_pair ( i -> name, p ) );
	}
}

void Gw :: Impl :: transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	EpMap :: iterator i = eps.find ( pdu.getEp ( ) );
	if ( i != eps.end ( ) ) {
		i -> second -> transactionTimedOut ( ret, pdu );
		return;
	}
	if ( pdu.getId ( ) == tid ) {
		if ( tries ++ <= 3 )
			sendAuep ( ret );
	} else
		PSYSTEMLOG ( Error, "transaction timeout to unknown endpoint " << pdu.getEp ( ) );
}

void Gw :: Impl :: detachThreadHandler ( MGCP :: PduVector & ret, const ss :: string & ep, bool ok ) {
	EpMap :: iterator i = eps.find ( ep );
	if ( i == eps.end ( ) ) {
		PSYSTEMLOG ( Error, "detachThreadHandler to unknown endpoint " << ep );
		return;
	}
	i -> second -> detachThreadHandler ( ret, ok );
}

bool Gw :: Impl :: originateCall ( const ss :: string & ep, const OriginateCallArg & a ) {
	EpMap :: iterator i = eps.find ( ep );
	if ( i == eps.end ( ) ) {
		PSYSTEMLOG ( Error, "originateCall to unknown endpoint " << ep );
		return false;
	}
	return i -> second -> originateCall ( a );
}

void Gw :: Impl :: sendRinging ( MGCP :: PduVector & ret, const ss :: string & ep, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec ) {
	EpMap :: iterator i = eps.find ( ep );
	if ( i == eps.end ( ) ) {
		PSYSTEMLOG ( Error, "sendRinging to unknown endpoint " << ep );
		return;
	}
	i -> second -> sendRinging ( ret, localRtpPort, localAddr, inCodec );
}

void Gw :: Impl :: sendOk ( MGCP :: PduVector & ret, const ss :: string & ep, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType ) {
	EpMap :: iterator i = eps.find ( ep );
	if ( i == eps.end ( ) ) {
		PSYSTEMLOG ( Error, "sendOk to unknown endpoint " << ep );
		return;
	}
	i -> second -> sendOk ( ret, localRtpPort, localAddr, inCodec, telephoneEventsPayloadType );
}

void Gw :: Impl :: handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const ss :: string & ep ) {
	EpMap :: iterator i = eps.find ( ep );
	if ( i != eps.end ( ) )
		i -> second -> handleResponse ( ret, pdu );
	else {
		if ( pdu.getId ( ) == tid )
			checkEndpoints ( pdu );
		else
			PSYSTEMLOG ( Error, "response to unknown endpoint " << ep );
	}
}

void Gw :: Impl :: checkEndpoints ( const MGCP :: PDU & pdu ) {
	StringVector zz;
	pdu.getMIME ( ).getSpecificEndPointId ( zz );
	StringVector zeps;
	zeps.reserve ( zz.size ( ) );
	for ( unsigned i = 0; i < zz.size ( ); i ++ ) {
		const ss :: string & z = zz [ i ];
		ss :: string :: size_type pos = z.find ( '@' );
		if ( pos == ss :: string :: npos ) {
			PSYSTEMLOG ( Error, "unsupported specific endpoint " << z );
			continue;
		}
		if ( z.compare ( pos + 1, ss :: string :: npos, name ) ) {
			PSYSTEMLOG ( Error, "unknown gateway " << z.substr ( pos + 1 ) );
			continue;
		}
		zeps.push_back ( z.substr ( 0, pos ) );
	}
	std :: sort ( zeps.begin ( ), zeps.end ( ) );
	StringVector o, n;
	set_threeway ( firster ( eps.begin ( ) ), firster ( eps.end ( ) ), zeps.begin ( ), zeps.end ( ),
		std :: back_inserter ( o ), boost :: function_output_iterator < Nop > ( ), std :: back_inserter ( n ) );
	for ( unsigned i = 0; i < o.size ( ); i ++ )
		PSYSTEMLOG ( Error, "device doesnt have endpoint " << o [ i ] );
	for ( unsigned i = 0; i < n.size ( ); i ++ )
		PSYSTEMLOG ( Error, "device have unknown endpoint " << n [ i ] );
}

static bool wildcarded ( const ss :: string & s ) {
	return s.find_first_of ( "*[" ) != ss :: string :: npos;
}

class EpMatcher : boost :: noncopyable { // ranges are not supported
	boost :: regex rx;
public:
	explicit EpMatcher ( const ss :: string & pattern ) {
		ss :: string s;
		for ( ss :: string :: size_type i = 0; i < pattern.size ( ); i ++ ) {
			switch ( pattern [ i ] ) {
				case '*':
					s += '.';
					break;
				case '[':
					PSYSTEMLOG ( Error, "possibly unsupported range wildcard " << pattern );
				case '.':
				case '{':
				case '(':
				case ')':
				case '\\':
				case '+':
				case '?':
				case '|':
				case '^':
				case '$':
					s += '\\';
					break;
			}
			s += pattern [ i ];
		}
		rx = s;
	}
	bool match ( const ss :: string & s ) const {
		try {
			return boost :: regex_match ( s, rx );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "exception when matching regex: " << e.what ( ) );
			return false;
		}
	}
};

void Gw :: Impl :: handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia ) {
	const ss :: string & ep = pdu.getEp ( );
	EpMap :: iterator i = eps.find ( ep );
	if ( i != eps.end ( ) )
		i -> second -> handleRequest ( ret, pdu, ia );
	else if ( wildcarded ( ep ) ) {
		if ( pdu.getVerb ( ) != MGCP :: PDU :: vRestartInProgress )
			makeResponse ( ret, pdu, MGCP :: PDU :: rcAllOfTooComplicated );
		else {
			ss :: string rm;
			pdu.getMIME ( ).getRestartMethod ( rm );
			int rd = 0;
			if ( rm == "restart" && ( ! pdu.getMIME ( ).getRestartDelay ( rd ) || ! rd ) ) {
				makeResponse ( ret, pdu, MGCP :: PDU :: rcOK );
				EpMatcher matcher ( ep );
				for ( i = eps.begin ( ); i != eps.end ( ); ++ i )
					if ( matcher.match ( i -> second -> getName ( ) ) )
						i -> second -> handleRestart ( ret );
			} else {
				PSYSTEMLOG ( Error, "unsupported restart method " << rm );
				makeResponse ( ret, pdu, MGCP :: PDU :: rcUnknownRestartMethod );
			}
		}
	} else {
		makeResponse ( ret, pdu, MGCP :: PDU :: rcUnknownEndpoint );
		PSYSTEMLOG ( Error, "request to unknown endpoint " << pdu.getEp ( ) );
	}
}

void Gw :: Impl :: sendAuep ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( "*", name, ip, port, MGCP :: PDU :: vAuditEndpoint );
	tid = pdu.getId ( );
	ret.push_back ( pdu );
}

void Gw :: Impl :: init ( MGCP :: PduVector & ret ) {
	sendAuep ( ret );
	for ( EpMap :: const_iterator i = eps.begin ( ); i != eps.end ( ); ++ i )
		i -> second -> init ( ret );
}

struct EpNameCmp {
	bool operator() ( const MgcpEndpointInfo & ep, const ss :: string & name ) {
		return ep.name < name;
	}
};

void Gw :: Impl :: reloadConf ( MGCP :: PduVector & ret, const MgcpGatewayInfo & gi, Gw * g ) {
	name = gi.name;
	ip = gi.ip;
	maxDatagram = gi.cls.maxDatagram;
	StringVector o, s, n;
	s.reserve ( gi.eps.size ( ) );
	//std::tr1::mem_fn ne rabotaet s boost::transform_iterator
 	set_threeway ( firster ( eps.begin ( ) ), firster ( eps.end ( ) ),
		make_transform_iterator ( boost :: mem_fn ( & MgcpEndpointInfo :: name ), gi.eps.begin ( ) ),
		make_transform_iterator ( boost :: mem_fn ( & MgcpEndpointInfo :: name ), gi.eps.end ( ) ),
		std :: back_inserter ( o ), std :: back_inserter ( s ), std :: back_inserter ( n ) );
	for ( std :: size_t i = o.size ( ); i > 0; )
		eps.erase ( o [ --i ] );
	for ( std :: size_t i = n.size ( ); i > 0; ) {
		const ss :: string & ename = n [ -- i ];
		const MgcpEndpointInfo & ei = * std :: lower_bound ( gi.eps.begin ( ), gi.eps.end ( ), ename, EpNameCmp ( ) );
		Pointer < Ep > p = new Ep ( ename, g, ei.inPeer, gi.cls );
		eps.insert ( std :: make_pair ( ename, p ) );
		p -> init ( ret );
	}
	for ( std :: size_t i = s.size ( ); i > 0; ) {
		const ss :: string & ename = s [ -- i ];
		const MgcpEndpointInfo & ei = * std :: lower_bound ( gi.eps.begin ( ), gi.eps.end ( ), ename, EpNameCmp ( ) );
		eps [ ename ] -> reloadConf ( ei.inPeer, gi.cls );
	}
}

Gw :: Gw ( const ss :: string & n, const ss :: string & a, const MgcpEndpointInfoVector & e, unsigned short p,
	const MgcpClassInfo & cl ) : impl ( new Impl ( this, impl, n, a, e, p, cl ) ) { }

Gw :: ~Gw ( ) {
	delete impl;
}

const ss :: string & Gw :: getName ( ) const {
	return impl -> getName ( );
}

const ss :: string & Gw :: getIp ( ) const {
	return impl -> getIp ( );
}

unsigned short Gw :: getPort ( ) const {
	return impl -> getPort ( );
}

unsigned short Gw :: getMaxDatagram ( ) const {
	return impl -> getMaxDatagram ( );
}

void Gw :: handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia ) {
	impl -> handleRequest ( ret, pdu, ia );
}

void Gw :: init ( MGCP :: PduVector & ret ) const {
	impl -> init ( ret );
}

void Gw :: handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const ss :: string & ep ) {
	impl -> handleResponse ( ret, pdu, ep );
}

void Gw :: transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	impl -> transactionTimedOut ( ret, pdu );
}

void Gw :: detachThreadHandler ( MGCP :: PduVector & ret, const ss :: string & ep, bool ok ) {
	impl -> detachThreadHandler ( ret, ep, ok );
}

bool Gw :: originateCall ( const ss :: string & ep, const OriginateCallArg & a ) {
	return impl -> originateCall ( ep, a );
}

void Gw :: sendRinging ( MGCP :: PduVector & ret, const ss :: string & ep, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec ) {
	impl -> sendRinging ( ret, ep, localRtpPort, localAddr, inCodec );
}

void Gw :: sendOk ( MGCP :: PduVector & ret, const ss :: string & ep, int localRtpPort,
	const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType ) {
	impl -> sendOk ( ret, ep, localRtpPort, localAddr, inCodec, telephoneEventsPayloadType );
}

void Gw :: reloadConf ( MGCP :: PduVector & ret, const MgcpGatewayInfo & gi ) {
	impl -> reloadConf ( ret, gi, this );
}

