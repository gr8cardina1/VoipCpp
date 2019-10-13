#pragma implementation
#pragma implementation "siporiginatehandler.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "codecinfo.hpp"
#include "signallingoptions.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "sourcedata.hpp"
#include "aftertask.hpp"
#include <ptlib.h>
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include <ptlib/sockets.h>
#include "q931.hpp"
#include "callcontrol.hpp"
#include "legthread.hpp"
#include <queue>
#include "originatelegthread.hpp"
#include "siporiginatehandler.hpp"
#include "sipleg2.hpp"
#include "sip2.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include "sdp.hpp"
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/range.hpp>
#include <ptlib/svcproc.h>
#include "slprint.hpp" // ss :: for_each
#include "sdpcommon.hpp"
#include "condvar.hpp"
#include "automutex.hpp"
#include "threadmessagequeue.hpp"
#include "siptransportthread.hpp"
#include "radgwinfo.hpp"
#include "icqcontact.hpp"
#include "ipport.hpp"
#include "Conf.hpp"
#include "dtmfrelay.hpp"

typedef std :: map < ss :: string, const RecodeInfo *, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, const RecodeInfo * > > > PayloadMap;

static void addFormats ( SDP :: MediaDescription & media, const RecodeInfoVector & codecs, PayloadMap & m ) {
	for ( RecodeInfoVector :: const_iterator i = codecs.begin ( ); i != codecs.end ( ); ++ i ) {
		try {
			const RecodeInfo & r = * i;
			ss :: string payload = addFormat ( media, r.codec );
			m.insert ( std :: make_pair ( payload, & r ) );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "esception: " << e.what ( ) );
		}
	}
}

static PIPSocket :: Address getLocalAddress ( ) {
	PIPSocket :: Address addr;
	PIPSocket :: GetHostAddress ( addr );
	return addr;
}

struct SipLeg2 :: Data {
	SIP2 :: ClientTransactionId inviteId, reinviteId;
	typedef std :: tr1 :: function < bool ( ) > SipMessage;
	ThreadMessageQueue < SipMessage > q;
	PayloadMap payloadMap;
	CodecInfo codec, changedCodec;
	Data ( ) : inviteId ( "", SIP2 :: Request :: mInvite ), reinviteId ( "", SIP2 :: Request :: mInvite ) { } // fake
};

namespace H323 {
ss :: string globallyUniqueId ( );
ss :: string printableCallId ( const ss :: string & confId );
}

bool SipLeg2 :: tryChoice ( ) {
	if ( ! SIP2 :: transportThread )
		return false;
	ss :: string host;
	ss :: string user;
	int port;
	ss :: string localHost;
	ss :: string localUser;
	ss :: string callId;

	common.sigOptions ( ).setOut ( common.curChoice ( ) -> getSigOptions ( ) );
	if ( common.getSigOptions ( ).isAccSend ( ) ) { // chto eto ?
		ss :: string accountCard;
		bool isCard = conf -> isRegisteredCardAddr ( common.curChoice ( ) -> getIp ( ),
			WORD ( common.curChoice ( ) -> getPort ( ) ), accountCard );
		if ( isCard )
			user = accountCard;
		else
			user = common.getSentDigits ( );
	} else
		user = common.getSentDigits ( ); // a eto nado

	PIPSocket :: Address addr = common.curChoice ( ) -> getInterfaces ( ) [ 0 ];
	if ( addr == INADDR_ANY )
		addr = getLocalAddress ( );
	localHost = static_cast < const char * > ( addr.AsString ( ) );
	host = common.curChoice ( ) -> getIp ( );
	port = common.curChoice ( ) -> getPort ( );

	if ( common.getCallingDigits ( ).empty ( ) )
		localUser = common.getCallingDigitsIn ( );
	else if ( common.getCallingDigits ( ) == "-" )
		localUser = "";
	else
		localUser = common.getCallingDigits ( );
	common.setConfId ( H323 :: globallyUniqueId ( ) );
	callId = H323 :: printableCallId ( common.getConfId ( ) );
	thread -> setPrintableCallId ( callId );
	thread -> calledSocketConnected ( addr );

	PIPSocket :: Address rtpAddr = thread -> getLocalIp ( );
	if ( rtpAddr == INADDR_ANY )
		rtpAddr = addr;
	ss :: string addrRTP = static_cast < const char * > ( rtpAddr.AsString ( ) );
	std :: auto_ptr < SDP :: SessionDescription > sdp ( new SDP :: SessionDescription ( addrRTP ) );
	sdp -> setConnectionAddress ( addrRTP );
	int rtpPort = thread -> getLocalPort ( );
	SDP :: MediaDescription media ( SDP :: mediaAudio, rtpPort, SDP :: protoRtpAvp );
	addFormats ( media, inRecodes, p -> payloadMap );
	if ( int telephoneEventsPayloadType = common.getTelephoneEventsPayloadType ( ) )
		addTelephoneEvents ( media, telephoneEventsPayloadType );
	sdp -> addMediaDescription ( media );
	StringVector realms;
	p -> inviteId = SIP2 :: transportThread -> originateCall ( host, user, port, localHost, localUser, callId,
		maxForwards, ++ lastCseq, realms, m_remotePartyID, sdp.release ( ), this, m_PAssertID, m_Privacy );
	return true;
}

bool SipLeg2 :: iteration ( ) {
	Data :: SipMessage m;
	if ( p -> q.get ( m ) )
		return m ( );
	return true;
}

void SipLeg2 :: closeChoice ( ) {
	SIP2 :: transportThread -> detachHandler ( p -> inviteId );
}

void SipLeg2 :: wakeUp ( ) {
	p -> q.wake ( );
}

void SipLeg2 :: tryingReceived ( ) {
	p -> q.put ( std :: tr1 :: bind ( & SipLeg2 :: tryingReceivedLocal, this ) );
}

bool SipLeg2 :: tryingReceivedLocal ( ) {
	thread -> answered ( );
	return true;
}

void SipLeg2 :: ringingReceived ( const SIP2 :: Response & r ) {
	p -> q.put ( std :: tr1 :: bind ( & SipLeg2 :: ringingReceivedLocal, this, r ) );
}

bool SipLeg2 :: ringingReceivedLocal ( const SIP2 :: Response & r ) {
	thread -> answered ( );
	handleAnswerSDP ( r );
	thread -> alerted ( );
	return true;
}

void SipLeg2 :: disconnectReceived ( int c, const SIP2 :: ResponseMIMEInfo & m ) {
	if ( c == SIP2 :: Response :: scRedirectionMovedTemporarily )
		p -> q.put ( std :: tr1 :: bind ( & SipLeg2 :: redirectReceivedLocal, this, c, m.getContact ( ) ) );
	else
		p -> q.put ( std :: tr1 :: bind ( & SipLeg2 :: disconnectReceivedLocal, this, c ) );
}

bool SipLeg2 :: disconnectReceivedLocal ( int c ) {
	int resp = 0;
	ss :: string textResp;
	conf -> getSIPToH323ErrorResponse ( c , & resp, & textResp );
	PSYSTEMLOG ( Info, "SipLeg2 :: disconnectReceived: " << c << "; text: " << conf -> getSIPText( c )
		<< "; Q931err: " << textResp << ", " << resp );
	if ( ! resp )
		resp = Q931 :: cvNoRouteToDestination;
	thread -> released ( Q931 :: CauseValues ( resp ) );
	return false;
}

bool SipLeg2 :: redirectReceivedLocal ( int /*c*/, const SIP2 :: ContactHeader & contact ) {
	if ( redirectCount ) { // TODO v 8.1.3.4 podrobno raspisano
		thread -> released ( Q931 :: cvRedirection );
		return false;
	}
	redirectCount ++;
	const SIP2 :: URI & u = contact.getUri ( );
	//TODO: nado iz contacta brat parametri, method i header ispolzovat po naznacheniyu, a ostalnoe peredavat v uri
	ss :: string host = u.getHost ( );
	ss :: string user = u.getUser ( );
	int port = u.getPort ( );
	ss :: string localHost;
	ss :: string localUser;
	ss :: string callId;

	PIPSocket :: Address addr = common.curChoice ( ) -> getInterfaces ( ) [ 0 ];
	if ( addr == INADDR_ANY )
		addr = getLocalAddress ( );
	localHost = static_cast < const char * > ( addr.AsString ( ) );

	if ( common.getCallingDigits ( ).empty ( ) )
		localUser = common.getCallingDigitsIn ( );
	else if ( common.getCallingDigits ( ) == "-" )
		localUser = "";
	else
		localUser = common.getCallingDigits ( );
	//TODO: It is RECOMMENDED that the UAC reuse the same To, From, and Call-ID
	// used in the original redirected request, but the UAC MAY also choose
	// to update the Call-ID header field value for new requests, for
	// example.
	// neponyatno kak pri etom dialogi otlichat
	common.setConfId ( H323 :: globallyUniqueId ( ) );
	callId = H323 :: printableCallId ( common.getConfId ( ) );
	thread -> setPrintableCallId ( callId );

	PIPSocket :: Address rtpAddr = thread -> getLocalIp ( );
	if ( rtpAddr == INADDR_ANY )
		rtpAddr = addr;
	ss :: string addrRTP = static_cast < const char * > ( rtpAddr.AsString ( ) );
	std :: auto_ptr < SDP :: SessionDescription > sdp ( new SDP :: SessionDescription ( addrRTP ) );
	sdp -> setConnectionAddress ( addrRTP );
	int rtpPort = thread -> getLocalPort ( );
	SDP :: MediaDescription media ( SDP :: mediaAudio, rtpPort, SDP :: protoRtpAvp );
 	addFormats ( media, inRecodes, p -> payloadMap );
	if ( int telephoneEventsPayloadType = common.getTelephoneEventsPayloadType ( ) )
		addTelephoneEvents ( media, telephoneEventsPayloadType );
	sdp -> addMediaDescription ( media );
	StringVector realms;
	p -> inviteId = SIP2 :: transportThread -> originateCall ( host, user, port, localHost, localUser, callId,
		maxForwards, ++ lastCseq, realms, m_remotePartyID, sdp.release ( ), this, m_PAssertID, m_Privacy );
	return true;
}

void SipLeg2 :: okReceived ( const SIP2 :: Response & r ) {
	p -> q.put ( std :: tr1 :: bind ( & SipLeg2 :: okReceivedLocal, this, r ) );
}

bool SipLeg2 :: okReceivedLocal ( const SIP2 :: Response & r ) {
	if ( ! handleAnswerSDP ( r ) )
		return false;
	thread -> connected ( );
	return true;
}

void SipLeg2 :: unauthorizedReceived ( const StringVector & realms ) {
	p -> q.put ( std :: tr1 :: bind ( & SipLeg2 :: unauthorizedReceivedLocal, this, realms ) );
}

bool SipLeg2 :: unauthorizedReceivedLocal ( const StringVector & realms ) {
	if ( gotUnauthorized )
		return disconnectReceivedLocal ( SIP2 :: Response :: scFailureUnAuthorized );
	gotUnauthorized = true;
	ss :: string host;
	ss :: string user;
	int port;
	ss :: string localHost;
	ss :: string localUser;
	ss :: string callId;

	if ( common.getSigOptions ( ).isAccSend ( ) ) { // chto eto ?
		ss :: string accountCard;
		bool isCard = conf -> isRegisteredCardAddr ( common.curChoice ( ) -> getIp ( ),
			WORD ( common.curChoice ( ) -> getPort ( ) ), accountCard );
		if ( isCard )
			user = accountCard;
		else
			user = common.getSentDigits ( );
	} else
		user = common.getSentDigits ( ); // a eto nado

	PIPSocket :: Address addr = common.curChoice ( ) -> getInterfaces ( ) [ 0 ];
	if ( addr == INADDR_ANY )
		addr = getLocalAddress ( );
	localHost = static_cast < const char * > ( addr.AsString ( ) );
	host = common.curChoice ( ) -> getIp ( );
	port = common.curChoice ( ) -> getPort ( );

	if ( common.getCallingDigits ( ).empty ( ) )
		localUser = common.getCallingDigitsIn ( );
	else if ( common.getCallingDigits ( ) == "-" )
		localUser = "";
	else
		localUser = common.getCallingDigits ( );
	common.setConfId ( H323 :: globallyUniqueId ( ) );
	callId = H323 :: printableCallId ( common.getConfId ( ) );
	thread -> setPrintableCallId ( callId );

	PIPSocket :: Address rtpAddr = thread -> getLocalIp ( );
	if ( rtpAddr == INADDR_ANY )
		rtpAddr = addr;
	ss :: string addrRTP = static_cast < const char * > ( rtpAddr.AsString ( ) );
	std :: auto_ptr < SDP :: SessionDescription > sdp ( new SDP :: SessionDescription ( addrRTP ) );
	sdp -> setConnectionAddress ( addrRTP );
	int rtpPort = thread -> getLocalPort ( );
	SDP :: MediaDescription media ( SDP :: mediaAudio, rtpPort, SDP :: protoRtpAvp );
	addFormats ( media, inRecodes, p -> payloadMap );
	if ( int telephoneEventsPayloadType = common.getTelephoneEventsPayloadType ( ) )
		addTelephoneEvents ( media, telephoneEventsPayloadType );
	sdp -> addMediaDescription ( media );
	SIP2 :: transportThread -> detachHandler ( p -> inviteId );
	p -> inviteId = SIP2 :: transportThread -> originateCall ( host, user, port, localHost, localUser, callId,
		maxForwards, ++ lastCseq, realms, m_remotePartyID, sdp.release ( ), this, m_PAssertID, m_Privacy );
	return true;
}

void SipLeg2 :: onholdReceived ( int level, int port, const ss :: string & addr ) {
	p -> q.put ( std :: tr1 :: bind ( & SipLeg2 :: onholdReceivedLocal, this, level, port, addr ) );
}

bool SipLeg2 :: onholdReceivedLocal ( int level, int port, const ss :: string & addr ) {
	thread -> setSendAddress ( PIPSocket :: Address ( addr.c_str ( ) ), port, p -> codec, p -> codec, p -> changedCodec,
			p -> changedCodec );
	thread -> onHold ( level );
	return true;
}

void SipLeg2 :: dtmfRelayReceived ( const DTMF :: Relay & r ) {
	p -> q.put ( std :: tr1 :: bind ( & SipLeg2 :: dtmfRelayReceivedLocal, this, r ) );
}

bool SipLeg2 :: dtmfRelayReceivedLocal ( const DTMF :: Relay & r ) {
	thread -> sendDtmf ( r );
	return true;
}

bool SipLeg2 :: peerOnHold ( int level ) {
	try {
		p -> reinviteId = SIP2 :: transportThread -> sendOnhold ( p -> inviteId, level, this );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "onhold: " << e.what ( ) );
	}
	return true;
}

bool SipLeg2 :: peerDtmf ( const DTMF :: Relay & r ) {
	try {
		SIP2 :: transportThread -> sendDtmf ( p -> inviteId, r );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "dtmf: " << e.what ( ) );
	}
	return true;
}

bool SipLeg2 :: acceptsDtmfOverride ( ) const {
	return common.getSigOptions ( ).getAceptedDTMF ( );
}

bool SipLeg2 :: handleAnswerSDP ( const SIP2 :: Response & r ) {
	const SDP :: SessionDescription * sdp = r.getSdp ( );
	if ( ! sdp )
		return false;
	const SDP :: MediaDescription * media = sdp -> getMediaDescription ( SDP :: mediaAudio );
	if ( ! media )
		return false;
	if ( unsigned payload = getTelephoneEventsPayloadType ( media ) )
		common.setTelephoneEventsPayloadType ( payload );
	if ( unsigned payload = common.getTelephoneEventsPayloadType ( ) )
		thread -> setTelephoneEventsPayloadType ( payload );
	const SDP :: MediaFormatVector & formats = media -> getFmts ( );
	unsigned ptime = getPtime ( media );
	for ( SDP :: MediaFormatVector :: const_iterator i = formats.begin ( ); i != formats.end ( ); ++ i ) {
		CodecInfo codec, changedCodec = getCodec ( * i, ptime );
		PayloadMap :: const_iterator j = p -> payloadMap.find ( i -> getFmt ( ) );
		if ( j == p -> payloadMap.end ( ) ) {
			PSYSTEMLOG ( Error, "payloadMap not found: " << * i );
			ss :: for_each ( p -> payloadMap, SLPrint ( "payloadMap: " ) );
			continue;
		}
		codec = j -> second -> backCodec;
		PIPSocket :: Address addr;
		if ( const ss :: string * ip = media -> getConnectionAddress ( ) )
			addr = ip -> c_str ( );
		else
			addr = sdp -> getConnectionAddress ( ).c_str ( );
		thread -> setCodec ( codec );
		thread -> setSendAddress ( addr, media -> getPort ( ), codec, codec, changedCodec, changedCodec );
		p -> codec = codec;
		p -> changedCodec = changedCodec;
		return true;
	}
	return false;
}

SipLeg2 :: SipLeg2 ( OriginateLegThread * t, CommonCallDetails & c, const RecodeInfoVector & inCodecs, unsigned maxForwards,
	const ss :: string & remotePartyID, const ss :: string & passertid, const ss :: string & privacy ) : Leg ( t, c ),
	inRecodes ( inCodecs ), m_remotePartyID ( common.getSigOptions ( ).getUseRemotePartyID ( ) ? remotePartyID : ss :: string ( ) ),
	m_Privacy ( privacy ), p ( new Data ), maxForwards ( maxForwards ), lastCseq ( 0 ), redirectCount ( 0 ),
	gotUnauthorized ( false ) {
	if ( common.getSigOptions ( ).isPAssertIdRequired ( ) ) {
		ss :: string pAssertID ( "\"" );
		pAssertID += c.getCallingDigits ( );
		pAssertID += "\" <sip:+";
		pAssertID += c.getCallingDigits ( ); //pomoemu eto dolgno bit getCallingDigitsIn ( )
		pAssertID += '@';
		pAssertID += common.getCallerIp ( );
		//naskolko ya ponimayu, eto dolgen bit ip originatora i ego luchshe iz pervogo via brat
		pAssertID += '>';
		m_PAssertID = pAssertID;
	} else
		m_PAssertID = passertid;
}

SipLeg2 :: ~SipLeg2 ( ) {
	delete p;
}
