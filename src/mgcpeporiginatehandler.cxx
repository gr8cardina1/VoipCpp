#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "mgcp.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "mgcpepstatehandler.hpp"
#include "mgcpeporiginatehandler.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include "mgcpep.hpp"
#include "mgcpepthreadhandler.hpp"
#include "pointer.hpp"
#include "mgcpsignalparser.hpp"
#include "ixcudpsocket.hpp"
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <ptlib/svcproc.h>
#include "mgcporiginatecallarg.hpp"
#include "mgcpepstarthandler.hpp"
#include "mgcpepnotifyhandler.hpp"
#include "sdp.hpp"
#include "sdpcommon.hpp"

EpOriginateHandler :: EpOriginateHandler ( Ep * e, const OriginateCallArg & a ) : EpStateHandler ( e ), handler ( a.th ),
	faState ( cCreateConnection ), retries ( 1 ), tid ( 0 ), hookUp ( true ), callId ( a.callId ), mediaIp ( a.mediaIp ),
	signalIp ( a.signalIp ), codec ( new CodecInfo ( a.codec ) ),
	telephoneEventsPayloadType ( a.telephoneEventsPayloadType ), rtpPort ( a.rtpPort ) {
	try {
		reSend ( a.ret );
	} catch ( ... ) {
		delete codec;
		throw;
	}
}

void EpOriginateHandler :: sendRqnt1 ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( ep -> getName ( ), ep -> getGwName ( ), ep -> getIp ( ), ep -> getPort ( ),
		MGCP :: PDU :: vNotificationRequest );
	tid = pdu.getId ( );
	MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	mime.setRequestIdentifier ( "0005" );
	mime.setRequestedEvents ( "L/hd(N)" );
	mime.setSignalRequests ( "L/rg" );
	ret.push_back ( pdu );
}

void EpOriginateHandler :: sendRqnt2 ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( ep -> getName ( ), ep -> getGwName ( ), ep -> getIp ( ), ep -> getPort ( ),
		MGCP :: PDU :: vNotificationRequest );
	tid = pdu.getId ( );
	MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	mime.setRequestIdentifier ( "0006" );
	mime.setRequestedEvents ( "L/hu(N)" );
	ret.push_back ( pdu );
}

void EpOriginateHandler :: sendCrcx ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( ep -> getName ( ), ep -> getGwName ( ), ep -> getIp ( ), ep -> getPort ( ),
		MGCP :: PDU :: vCreateConnection );
	tid = pdu.getId ( );
	MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	mime.setCallId ( callId );
	mime.setMode ( "sendrecv" );
//	mime.setLocalConnectionOptions ( "p:20, a:G729" );
	std :: auto_ptr < SDP :: SessionDescription > sdp ( new SDP :: SessionDescription ( signalIp ) );
	sdp -> setConnectionAddress ( mediaIp );
	SDP :: MediaDescription media ( SDP :: mediaAudio, rtpPort, SDP :: protoRtpAvp );
	addFormat ( media, * codec );
	if ( telephoneEventsPayloadType )
		addTelephoneEvents ( media, telephoneEventsPayloadType );
	sdp -> addMediaDescription ( media );
	pdu.setSDP ( sdp.release ( ) );
	ret.push_back ( pdu );
}

void EpOriginateHandler :: reSend ( MGCP :: PduVector & ret ) {
	switch ( faState ) {
		case cCreateConnection:
			sendCrcx ( ret );
			break;
		case cNotifyRequest1:
			sendRqnt1 ( ret );
			break;
		case cNotifyRequest2:
			sendRqnt2 ( ret );
			break;
		default:
			* ( int * ) 0 = 1;
	}
}

void EpOriginateHandler :: handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	if ( tid != pdu.getId ( ) ) {
		PSYSTEMLOG ( Error, "unknown transaction response: " << pdu.getId ( ) );
		EpStateHandler :: handleResponse ( ret, pdu );
		return;
	}
	if ( pdu.getCode ( ) != MGCP :: PDU :: rcOK ) {
		PSYSTEMLOG ( Error, "unknown result code: " << int ( pdu.getCode ( ) ) );
		return;
	}
	retries = 1;
	switch ( faState ) {
		case cCreateConnection:
			if ( const SDP :: SessionDescription * sdp = pdu.getSDP ( ) )
				handler -> sdpAck ( * sdp );
			else {
				PSYSTEMLOG ( Error, "no sdp after crcx" );
				ep -> changeHandler ( new EpNotifyHandler ( ep, ret, true, true ) );
				return;
			}
			faState = cNotifyRequest1;
			reSend ( ret );
			return;
		case cNotifyRequest1:
			faState = cWaitForNotify;
			return;
		case cNotifyRequest2:
			faState = cWaitForDetach;
			return;
		default:
			* ( int * ) 0 = 1;
	}
}

void EpOriginateHandler :: handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & ia ) {
	if ( pdu.getVerb ( ) != MGCP :: PDU :: vNotify ) {
		PSYSTEMLOG ( Error, "unknown request: " << int ( pdu.getVerb ( ) ) );
		EpStateHandler :: handleRequest ( ret, pdu, ia );
		return;
	}
	ss :: string oe;
	const MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	if ( ! mime.getObservedEvents ( oe ) ) {
		PSYSTEMLOG ( Error, "no observed events" );
		EpStateHandler :: handleRequest ( ret, pdu, ia );
		return;
	}
	MGCP :: SignalRequestVector events ( MGCP :: parseSignalRequests ( oe ) );
	if ( events.empty ( ) ) {
		PSYSTEMLOG ( Error, "parsed empty observed events list" );
		EpStateHandler :: handleRequest ( ret, pdu, ia );
		return;
	}
	makeResponse ( ret, pdu, MGCP :: PDU :: rcOK );
	bool hookChanged = false;
	for ( size_t i = 0; i < events.size ( ); i ++ ) {
		const MGCP :: EventName en = events [ i ].getName ( );
		PSYSTEMLOG ( Info, "handling event: " << events [ i ] );
		if ( en.getPackage ( ) == "L" && en.getEvent ( ) == "hd" ) {
			hookChanged = true;
			continue;
		}
		if ( en.getPackage ( ) == "L" && en.getEvent ( ) == "hu" ) {
			ep -> changeHandler ( new EpStartHandler ( ep, ret, true ) );
			return;
		}
		PSYSTEMLOG ( Error, "neponyatniy event" );
	}
	if ( faState != cWaitForNotify ) {
		PSYSTEMLOG ( Error, "notify in unknown state " << int ( faState ) );
		ep -> changeHandler ( new EpStartHandler ( ep, ret, true ) );
		return;
	}
	if ( hookChanged ) {
		hookUp = false;
		faState = cNotifyRequest2;
		handler -> connected ( );
	} else
		faState = cNotifyRequest1;
	reSend ( ret );
}

void EpOriginateHandler :: transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	if ( pdu.getId ( ) != tid )
		PSYSTEMLOG ( Error, "unknown transaction timeout: " << pdu.getId ( ) );
	if ( ++ retries <= 3 )
		reSend ( ret );
	else {
		ep -> changeHandler ( new EpStartHandler ( ep, ret, true ) );
	}
}

EpOriginateHandler :: ~EpOriginateHandler ( ) {
	delete codec;
	handler -> detach ( );
}
