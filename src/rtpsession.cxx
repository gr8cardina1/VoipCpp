#pragma implementation
#pragma implementation "session.hpp"
#pragma implementation "rtprequest.hpp"
#include <ptlib.h>
#include <ptlib/pipechan.h>
#include "ss.hpp"
#include "allocatable.hpp"
#include "rtppipe.hpp"
#include "rtpstat.hpp"
#include "rtprequest.hpp"
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "session.hpp"
#include "rtpsession.hpp"
#include <ptlib/svcproc.h>
#include "automutex.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include <cstring>

extern RTPPipe * rtpPipe;

RTPSession :: RTPSession ( RTPStat * st, const PIPSocket :: Address & from, const PIPSocket :: Address & to,
	int fromNat, bool toNat ) : Session ( st ), recoders ( conf -> getRecoders ( ) ), id ( - 1 ) {
	RTPRequest r ( rqAlloc );
	r.data [ 0 ] = from;
	r.data [ 1 ] = to;
	r.data [ 2 ] = fromNat;
	r.data [ 3 ] = toNat;
	if ( ! rtpPipe -> send ( r, 5 ) )
		return;
	id = r.data [ 0 ];
	if ( id == - 1 )
		return;
	for ( int i = 0; i < 4; i ++ ) {
		ports [ i ] = r.data [ i + 1 ];
		if ( ! ports [ i ] )
			PSYSTEMLOG ( Error, "RTPSession: ports[" << i << "]==0 !!!!!!!!" );
	}
}

RTPSession :: ~RTPSession ( ) {
	if ( id == - 1 )
		return;
	RTPRequest r ( rqFree );
	r.data [ 0 ] = id;
	rtpPipe -> send ( r, sizeof ( * stats ) * 4 / sizeof ( int ) );
	std :: memcpy ( stats, r.data, sizeof ( * stats ) * 4 );
}

int RTPSession :: getLocalAddress ( bool fromCaller, bool fControlChannel ) {
	int first = fromCaller ? 2 : 0;
	int second = fControlChannel ? 1 : 0;
	if ( ! ports [ first + second ] )
		PSYSTEMLOG ( Error, "getLocalAddress(" << fromCaller << ',' << fControlChannel << ',' << id << ")==0 !!!!!" );
	return ports [ first + second ];
}

void RTPSession :: setSendAddress ( bool fromCaller, bool fControlChannel, const PIPSocket :: Address & addr,
	WORD port, int rtpCodec, int rtpFrames, bool sendCodec, const ss :: string & recodeTo,
	const ss :: string & recodeFrom ) {
	PSYSTEMLOG ( Info, "RTPSession :: setSendAddress: addr = " << addr << ", port = " << port <<
		"; caller = " << fromCaller << "; fControlChannel = " << fControlChannel << "; rtpFrames = " << rtpFrames );
	if ( id == - 1 )
		return;
	if ( sendCodec && ( rtpCodec == 1 || rtpCodec == 2 ) ) {
		if ( rtpFrames >= 80 )
			rtpFrames /= 80;
		else if ( rtpFrames >= 10 )
			rtpFrames /= 10;
	}
	RTPRequest r ( rqSetSendAddress );
	r.data [ 0 ] = id * 4 + fromCaller * 2 + fControlChannel;
	r.data [ 1 ] = addr;
	r.data [ 2 ] = port;
	r.data [ 3 ] = rtpCodec;
	r.data [ 4 ] = rtpFrames;
	r.data [ 5 ] = sendCodec;
	r.data [ 6 ] = 0;

	if ( ! recodeTo.empty ( ) && ! CodecInfo :: isTrivialRecoder ( recodeFrom, recodeTo ) ) {
		RecodersMap :: const_iterator i = recoders.find ( recodeFrom );
		if ( i != recoders.end ( ) ) {
			IpPortMap :: const_iterator j = i -> second.find ( recodeTo );
			if ( j != i -> second.end ( ) ) {
				r.data [ 6 ] = j -> second.port;
				r.data [ 7 ] = PIPSocket :: Address ( j -> second.ip.c_str ( ) );
			} else
				PSYSTEMLOG ( Error, "no recoder from " << recodeFrom << " to " << recodeTo );
		} else
			PSYSTEMLOG ( Error, "no recoder from " << recodeFrom );
	}

	for(int i = 0; i < 4; ++i)
	{
		if(ports[i] == port)
		{
			PSYSTEMLOG(Error, "Port number is not unique.");
			static PTCPSocket is;
			PIPSocket :: InterfaceTable it;

			if ( ! is.GetInterfaceTable ( it ) ) {
				PSYSTEMLOG ( Error, "GetInterfaceTable: " << is.GetErrorText ( ) );
				return;
			}
			for ( int j = 0; j < it.GetSize ( ); ++ j )
			{
				PSYSTEMLOG ( Info, "Iface: " << it [ j ] );
				if ( addr == it [ j ].GetAddress() )
				{
					PSYSTEMLOG(Error, "Address is present in interface table. In this Call would be not present RTP.");
					return;
				}
			}

			break;
		}
	}

	rtpPipe -> send ( r );
}

void RTPSession :: setTelephoneEventsPayloadType(/*bool fromCaller, bool fControlChannel, */
				unsigned telephoneEventsPayloadType)
{
	PSYSTEMLOG ( Info, "RTPSession :: setTelephoneEventsCodec: codecTelephoneEvents = " << telephoneEventsPayloadType );
	RTPRequest r ( rqSetTelephoneEventsPayloadType );
	r.data [ 0 ] = id;// * 4 + fromCaller * 2;// + fControlChannel;
	r.data [ 1 ] = telephoneEventsPayloadType;

	rtpPipe -> send ( r );
}


void RTPSession :: getTimeout ( int & i, int & o ) {
	i = o = std :: numeric_limits < int > :: max ( );
	if ( id == - 1 )
		return;
	RTPRequest r ( rqGetTimeout );
	r.data [ 0 ] = id;
	if ( ! rtpPipe -> send ( r, 2 ) )
		return;
	i = r.data [ 0 ];
	o = r.data [ 1 ];
	return;
}

void RTPSession :: enableLog ( ) {
	if ( id == - 1 )
		return;
	RTPRequest r ( rqEnableLog );
	r.data [ 0 ] = id;
	rtpPipe -> send ( r );
}

void RTPSession :: setTo ( const PIPSocket :: Address & addr, bool toNat ) {
	if ( id == - 1 )
		return;
	RTPRequest r ( rqSetTo );
	r.data [ 0 ] = id;
	r.data [ 1 ] = addr;
	r.data [ 2 ] = toNat;
	rtpPipe -> send ( r, 2 );
	ports [ 2 ] = r.data [ 0 ];
	ports [ 3 ] = r.data [ 1 ];
}

void RTPSession :: setFrom ( const PIPSocket :: Address & addr, bool fromNat ) {
	if ( id == - 1 )
		return;
	RTPRequest r ( rqSetFrom );
	r.data [ 0 ] = id;
	r.data [ 1 ] = addr;
	r.data [ 2 ] = fromNat;
	rtpPipe -> send ( r, 2 );
	ports [ 0 ] = r.data [ 0 ];
	ports [ 1 ] = r.data [ 1 ];
}

std :: ostream & operator<< ( std :: ostream & os, const RTPRequest & r ) {
	switch ( r.type ) {
		case rqAlloc:
			return os << "rqAlloc from:" << PIPSocket :: Address ( r.data [ 0 ] ) << " to:" <<
				PIPSocket :: Address ( r.data [ 1 ] ) << " fromNat:" << r.data [ 2 ] <<
				" toNat:" << r.data [ 3 ];
		case rqFree:
			return os << "rqFree id:" << r.data [ 0 ];
		case rqSetSendAddress:
			return os << "rqSetSendAddress channel:" << r.data [ 0 ] << " addr:" <<
				PIPSocket :: Address ( r.data [ 1 ] ) << " port:" << r.data [ 2 ] << " rtpCodec:" <<
				r.data [ 3 ] << " rtpFrames:" << r.data [ 4 ] << " sendCodec:" << r.data [ 5 ] <<
				" recoderPort:" << r.data [ 6 ] << " recoderAddr:" <<
				PIPSocket :: Address ( r.data [ 7 ] );
		case rqInit:
			return os << "rqInit";
		case rqGetTimeout:
			return os << "rqGetTimeout id:" << r.data [ 0 ];
		case rqEnableLog:
			return os << "rqEnableLog id:" << r.data [ 0 ];
		case rqSetTo:
			return os << "rqSetTo id:" << r.data [ 0 ] << " addr:" <<
				PIPSocket :: Address ( r.data [ 1 ] ) << " toNat:" << r.data [ 2 ];
		case rqSetFrom:
			return os << "rqSetFrom id:" << r.data [ 0 ] << " addr:" <<
				PIPSocket :: Address ( r.data [ 1 ] ) << " toNat:" << r.data [ 2 ];
		case rqSetTelephoneEventsPayloadType:
			return os << "rqSetTelephoneEventsPayloadType";

	}
	return os << "rqUnknown";
}

const int sessionsNum = 2048;
static void * sessionArray [ sessionsNum ];
static PMutex mut;
void * RTPSession :: operator new ( std :: size_t n ) {
	AutoMutex am ( mut );
	for ( int i = 0; i < sessionsNum; i ++ )
		if ( sessionArray [ i ] == 0 )
			return sessionArray [ i ] = Session :: operator new ( n );
	throw std :: bad_alloc ( );
}

void RTPSession :: operator delete ( void * p, std :: size_t n ) throw ( ) {
	AutoMutex am ( mut );
	for ( int i = 0; i < sessionsNum; i ++ )
		if ( sessionArray [ i ] == p ) {
			sessionArray [ i ] = 0;
			break;
		}
	Session :: operator delete ( p, n );
}

