#pragma implementation
#pragma implementation "sipanswerhandler.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include <boost/archive/text_oarchive.hpp>
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
#include "q931.hpp"
#include <ptlib/sockets.h>
#include "callcontrol.hpp"
#include "legthread.hpp"
#include "answerlegthread.hpp"
#include "sipanswerhandler.hpp"
#include "sipanswerleg2.hpp"
#include "rtpstat.hpp"
#include "session.hpp"
#include "ipport.hpp"
#include "rtpsession.hpp"
#include "h323call.hpp"
#include <queue>
#include "condvar.hpp"
#include "automutex.hpp"
#include "threadmessagequeue.hpp"
#include "sip2.hpp"
#include "siptransportthread.hpp"
#include <ptlib/svcproc.h>

#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "dtmfrelay.hpp"


struct SipAnswerLegThread2 :: Data {
	ThreadMessageQueue < PeerMessage > q;
	SIP2 :: ServerTransactionId id;
	SIP2 :: ClientTransactionId reinviteId;
	explicit Data ( const SIP2 :: ServerTransactionId & id ) : id ( id ), reinviteId ( "", SIP2 :: Request :: mInvite ) { }
};

void SipAnswerLegThread2 :: putMessage ( const PeerMessage & m ) {
	p -> q.put ( m );
}

void SipAnswerLegThread2 :: disconnectReceived ( int c ) {
	p -> q.put ( std :: tr1 :: bind ( & SipAnswerLegThread2 :: disconnectReceivedLocal, this, c ) );
}

void SipAnswerLegThread2 :: disconnectReceivedLocal ( int /*c*/ ) {
	detached = true;
	common.setDisconnectCauseWeak ( Q931 :: cvNormalCallClearing ); // tut mogno po tablichke prognat
	call -> released ( common.getDisconnectCause ( ) );
}

void SipAnswerLegThread2 :: cancelReceived ( ) {
	p -> q.put ( std :: tr1 :: bind ( & SipAnswerLegThread2 :: cancelReceivedLocal, this ) );
}

void SipAnswerLegThread2 :: cancelReceivedLocal ( ) {
	if ( oked || detached )
		return;
	detached = true;
	common.setDisconnectCauseWeak ( Q931 :: cvNormalCallClearing );
	call -> released ( common.getDisconnectCause ( ) );
	SIP2 :: transportThread -> detachHandler ( p -> id, SIP2 :: Response :: scFailureRequestTerminated );
}

void SipAnswerLegThread2 :: onholdReceived ( int level, int port, const ss :: string & addr ) {
	p -> q.put ( std :: tr1 :: bind ( & SipAnswerLegThread2 :: onholdReceivedLocal, this, level, port, addr ) );
}

void SipAnswerLegThread2 :: onholdReceivedLocal ( int level, int port, const ss :: string & addr ) {
	CodecInfo dummy;
	call -> setSendAddress ( this, PIPSocket :: Address ( addr.c_str ( ) ), port, dummy, dummy, dummy, dummy );
	call -> onHold ( this, level );
}

void SipAnswerLegThread2 :: dtmfRelayReceived ( const DTMF :: Relay & r ) {
	p -> q.put ( std :: tr1 :: bind ( & SipAnswerLegThread2 :: dtmfRelayReceivedLocal, this, r ) );
}

void SipAnswerLegThread2 :: dtmfRelayReceivedLocal ( const DTMF :: Relay & r ) {
	call -> sendDtmf ( this, r );
}

bool SipAnswerLegThread2 :: iteration ( ) {
	PeerMessage m;
	if ( p -> q.get ( m ) )
		m ( );
	if ( ! oked && expires && startTime + expires * 1000ll <= PTime ( ) ) {
		SIP2 :: transportThread -> detachHandler ( p -> id, SIP2 :: Response :: scFailureRequestTerminated );
		detached = true;
		common.setDisconnectCauseWeak ( Q931 :: cvRecoveryOnTimerExpires );
		call -> released ( common.getDisconnectCause ( ) );
		return false;
	}
	return true;
}

bool SipAnswerLegThread2 :: ended ( ) const {
	return detached;
}

void SipAnswerLegThread2 :: shutDownLeg ( ) {
	if ( detached )
		return;
	int response = 0;
	ss :: string textResp;
	conf -> getH323ToSIPErrorResponse ( common.getDisconnectCause ( ), & response, & textResp );
	PSYSTEMLOG ( Info, "SipAnswerLegThread2 :: shutDownLeg. " << common.getDisconnectCause ( ) <<
		"; " << response << "; " << textResp );
	SIP2 :: transportThread -> detachHandler ( p -> id, response );
	detached = true;
}

void SipAnswerLegThread2 :: init ( ) { }

void SipAnswerLegThread2 :: peerAlertedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & /*outCodec*/ ) {
	SIP2 :: transportThread -> sendRinging ( p -> id, localRtpPort, localAddr, inCodec );
}

void SipAnswerLegThread2 :: peerConnectedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & /*outCodec*/ ) {
	oked = true;
	SIP2 :: transportThread -> sendOk ( p -> id, localRtpPort, localAddr, inCodec, common.getTelephoneEventsPayloadType ( ) );
}

void SipAnswerLegThread2 :: peerOnHoldLeg ( int level ) {
	try {
		p -> reinviteId = SIP2 :: transportThread -> sendOnhold ( p -> id, level, this );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "onhold: " << e.what ( ) );
	}
}

void SipAnswerLegThread2 :: peerDtmfLeg ( const DTMF :: Relay & r ) {
	try {
		SIP2 :: transportThread -> sendDtmf ( p -> id, r );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "dtmf: " << e.what ( ) );
	}
}

SipAnswerLegThread2 :: SipAnswerLegThread2 ( H323Call * c, LegThread * * p, unsigned i, const CommonCallDetails & d,
	const SIP2 :: ServerTransactionId & id, unsigned expires ) : AnswerLegThread ( c, p, i, ccd ), ccd ( d ),
	p ( new Data ( id ) ), expires ( expires ), lastCseq ( 0 ), oked ( false ), detached ( false ) {
	Resume ( );
}

SipAnswerLegThread2 :: ~SipAnswerLegThread2 ( ) {
	delete p;
}
