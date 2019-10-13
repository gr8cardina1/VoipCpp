#pragma implementation
#pragma implementation "mgcpepthreadhandler.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include "mgcp.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "mgcpepstatehandler.hpp"
#include "mgcpepcallhandler.hpp"
#include "mgcpep.hpp"
#include "mgcpepnotifyhandler.hpp"
#include <ptlib/svcproc.h>
#include "mgcpepthreadhandler.hpp"
namespace H323 {
	ss :: string globallyUniqueId ( ); //#include "h323.hpp"
}
#include "sipcommon.hpp" // for toHex
#include "signallingoptions.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "codecinfo.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "sourcedata.hpp"
#include "pointer.hpp"
#include "aftertask.hpp"
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "ipport.hpp"
#include "Conf.hpp"
#include "session.hpp"
#include "rtpsession.hpp"
#include "rtpstat.hpp"
#include "q931.hpp"
#include "h323call.hpp"
#include "answercall.hpp"
#include "mgcpepstarthandler.hpp"
#include "ixcudpsocket.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include "sdp.hpp"
#include "sdpcommon.hpp"
#include <queue>
//-----

EpThreadHandler :: ~EpThreadHandler ( ) { }

struct EpCallHandler :: Data {
	CodecInfo answeredCodec;
	struct Command {
		std :: tr1 :: function < void ( MGCP :: PduVector & ) > f;
		State allowedState;
		template < class T > Command ( const T & fu, State s ) : f ( fu ),
			allowedState ( s ) { }
	};
	std :: queue < Command, std :: deque < Command, __SS_ALLOCATOR < Command > > > q;
	PTime initTime;
};

void EpCallHandler :: sendRqnt ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( ep -> getName ( ), ep -> getGwName ( ), ep -> getIp ( ), ep -> getPort ( ),
		MGCP :: PDU :: vNotificationRequest );
	tid = pdu.getId ( );
	MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	mime.setRequestIdentifier ( "0003" );
	mime.setRequestedEvents ( "L/hu(N)" );
	ret.push_back ( pdu );
}

void EpCallHandler :: sendCrcx ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( ep -> getName ( ), ep -> getGwName ( ), ep -> getIp ( ), ep -> getPort ( ),
		MGCP :: PDU :: vCreateConnection );
	tid = pdu.getId ( );
	MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	mime.setCallId ( callId );
	mime.setMode ( "recvonly" );
	mime.setLocalConnectionOptions ( "p:20, a:G729" );
	ret.push_back ( pdu );
}

void EpCallHandler :: sendMdcx ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( ep -> getName ( ), ep -> getGwName ( ), ep -> getIp ( ), ep -> getPort ( ),
		MGCP :: PDU :: vModifyConnection );
	tid = pdu.getId ( );
	MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	mime.setCallId ( callId );
	mime.setMode ( "recvonly" );
//	mime.setLocalConnectionOptions ( "p:20, a:G729" );
	mime.setConnectionId ( connectionId );
	if ( data -> answeredCodec.getCodec ( ) != "unknown" ) {
		std :: auto_ptr < SDP :: SessionDescription > sdp ( new SDP :: SessionDescription ( answeredAddr ) );
		sdp -> setConnectionAddress ( answeredAddr );
		SDP :: MediaDescription media ( SDP :: mediaAudio, answeredPort, SDP :: protoRtpAvp );
		addFormat ( media, data -> answeredCodec );
		sdp -> addMediaDescription ( media );
		pdu.setSDP ( sdp.release ( ) );
	}
	ret.push_back ( pdu );
}

void EpCallHandler :: sendRinging ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( ep -> getName ( ), ep -> getGwName ( ), ep -> getIp ( ), ep -> getPort ( ),
		MGCP :: PDU :: vNotificationRequest );
	tid = pdu.getId ( );
	MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	mime.setRequestIdentifier ( "0004" );
	mime.setRequestedEvents ( "L/hu(N)" );
	mime.setSignalRequests ( "L/rg" );
	ret.push_back ( pdu );
}

void EpCallHandler :: sendMdcx2 ( MGCP :: PduVector & ret ) {
	MGCP :: PDU pdu ( ep -> getName ( ), ep -> getGwName ( ), ep -> getIp ( ), ep -> getPort ( ),
		MGCP :: PDU :: vModifyConnection );
	tid = pdu.getId ( );
	MGCP :: MIMEInfo & mime = pdu.getMIME ( );
	mime.setCallId ( callId );
	mime.setMode ( "sendrecv" );
	mime.setConnectionId ( connectionId ); // a tut vrode ni sdp ni localopts ne nado.
	std :: auto_ptr < SDP :: SessionDescription > sdp ( new SDP :: SessionDescription ( answeredAddr ) );
	sdp -> setConnectionAddress ( answeredAddr ); // nado sdelat razdelenie na media i signal kak v originatehandler
	SDP :: MediaDescription media ( SDP :: mediaAudio, answeredPort, SDP :: protoRtpAvp );
	addFormat ( media, data -> answeredCodec );
	if ( answeredTelephoneEventsPayloadType )
		addTelephoneEvents ( media, answeredTelephoneEventsPayloadType );
	sdp -> addMediaDescription ( media );
	pdu.setSDP ( sdp.release ( ) );
	ret.push_back ( pdu );
}

void EpCallHandler :: reSend ( MGCP :: PduVector & ret ) {
	switch ( faState ) {
		case cNotify:
			sendRqnt ( ret );
			break;
		case cCreateConnection:
			sendCrcx ( ret );
			break;
		case cModify:
			sendMdcx ( ret );
			break;
		case cRinging:
			sendRinging ( ret );
			break;
		case cModify2:
			sendMdcx2 ( ret );
			break;
		default:
			* ( int * ) 0 = 1;
	}
}

void EpCallHandler :: handleResponse ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
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
		case cNotify:
			faState = cCreateConnection;
			reSend ( ret );
			return;
		case cCreateConnection:
			createCall ( ret, pdu );
			return;
		case cModify:
			faState = cRinging;
			reSend ( ret );
			return;
		case cRinging:
			faState = cWaitForModify2;
			break;
		case cModify2:
			faState = cWaitForDetach;
			break;
		default:
			* ( int * ) 0 = 1;
	}
	checkDeferredCommands ( ret );
}

void EpCallHandler :: checkDeferredCommands ( MGCP :: PduVector & ret ) {
	while ( ! data -> q.empty ( ) ) {
		const Data :: Command & c = data -> q.front ( );
		if ( c.allowedState != faState )
			return;
		c.f ( ret );
		data -> q.pop ( );
	}
}

EpCallHandler :: ~EpCallHandler ( ) {
	if ( handler )
		handler -> detach ( );
	delete data;
}

static ss :: string newCallId ( ) {
	return toHex ( H323 :: globallyUniqueId ( ) );
}

EpCallHandler :: EpCallHandler ( Ep * e, MGCP :: PduVector & ret, const ss :: string & dd, const PIPSocket :: Address & ia ) :
	EpStateHandler ( e ), dialedDigits ( dd ), callId ( newCallId ( ) ), handler ( 0 ), ifAddr ( ia ), tid ( 0 ),
	answeredTelephoneEventsPayloadType ( 0 ), retries ( 1 ), faState ( cNotify ) {
	reSend ( ret );
	data = new Data;
}

void EpCallHandler :: transactionTimedOut ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	if ( pdu.getId ( ) != tid )
		PSYSTEMLOG ( Error, "unknown transaction timeout: " << pdu.getId ( ) );
	if ( ++ retries <= 3 )
		reSend ( ret );
	else {
		ep -> changeHandler ( new EpNotifyHandler ( ep, ret, false, false ) );
	}
}

void EpCallHandler :: handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & /*pdu*/, const PIPSocket :: Address & /*ia*/ ) {
	ep -> changeHandler ( new EpNotifyHandler ( ep, ret, true, false ) );
}


void EpCallHandler :: createCall ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu ) {
	pdu.getMIME ( ).getConnectionId ( connectionId );
	if ( ! pdu.getSDP ( ) ) {
		PSYSTEMLOG ( Error, "no sdp after crcx" );
		ep -> changeHandler ( new EpNotifyHandler ( ep, ret, false, false ) );
		return;
	}
	int inPeer = ep -> getInPeer( );
	if ( ! inPeer ) {
		PSYSTEMLOG ( Error, "no inPeer on endpoint" );
		ep -> changeHandler ( new EpNotifyHandler ( ep, ret, false, false ) );
		return;
	}
	if ( ! conf -> tryTake ( ) ) {
		PSYSTEMLOG ( Error, "tryTake failed" );
		ep -> changeHandler ( new EpNotifyHandler ( ep, ret, false, false ) );
		return;
	}
	CommonCallDetails common;
	common.setFromNat ( false );
	common.setCallerIp ( pdu.getIp ( ) );
	common.setCallerPort ( pdu.getPort ( ) );
	common.setConfId ( callId );
	if ( conf -> isValidInPeerAddress ( common.getCallerIp ( ) ) )
		common.source ( ).type = SourceData :: inbound;
	common.setDialedDigits ( dialedDigits );
	const SDP :: SessionDescription & sdp = * pdu.getSDP ( );
	getIncomingCodecsFromSDP ( sdp, common.incomingCodecs ( ) );
	OutChoiceDetailsVectorVector choiceForks;
	StringVector forkOutAcctns;
	if ( ! conf -> getCallInfo ( choiceForks, forkOutAcctns, common, true, true, true ) ||
		common.getIncomingCodecs ( ).c.empty ( ) ) {
		common.release ( );
		ep -> changeHandler ( new EpNotifyHandler ( ep, ret, false, false ) );
		return;
	}
	faState = cWaitForModify;
	EpThreadHandler * t = 0;
	new AnswerCall ( choiceForks, forkOutAcctns, common, sdp, t, ep -> getGwName ( ), ep -> getName ( ), data -> initTime );
	handler = t;
	return;
}

void EpCallHandler :: sendRinging ( MGCP :: PduVector & ret, int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec ) {
	if ( faState != cWaitForModify ) {
		PSYSTEMLOG ( Error, "sendRinging in state " << faState );
		ep -> changeHandler ( new EpStartHandler ( ep, ret, false ) );
		return;
	}
	data -> answeredCodec = inCodec;
	answeredPort = localRtpPort;
	answeredAddr = static_cast < const char * > ( ( localAddr == INADDR_ANY ? ifAddr : localAddr ).AsString ( ) );
	faState = cModify;
	retries = 1;
	reSend ( ret );
}

void EpCallHandler :: sendOk ( MGCP :: PduVector & ret, int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, unsigned telephoneEventsPayloadType ) {
	if ( faState != cWaitForModify2 && faState != cWaitForModify ) {
		if ( faState == cModify || faState == cRinging ) {
			PSYSTEMLOG ( Info, "sendOk in state " << faState << ", deferring" );
			data -> q.push ( Data :: Command ( std :: tr1 :: bind ( & EpCallHandler :: sendOk, this,
				std :: tr1 :: placeholders :: _1, localRtpPort, localAddr, inCodec,
				telephoneEventsPayloadType ), cWaitForModify2 ) );
			return;
		}
		PSYSTEMLOG ( Error, "sendOk in state " << faState );
		ep -> changeHandler ( new EpStartHandler ( ep, ret, false ) );
		return;
	}
	data -> answeredCodec = inCodec;
	answeredPort = localRtpPort;
	answeredAddr = static_cast < const char * > ( ( localAddr == INADDR_ANY ? ifAddr : localAddr ).AsString ( ) );
	answeredTelephoneEventsPayloadType = telephoneEventsPayloadType;
	faState = cModify2;
	retries = 1;
	reSend ( ret );
}
