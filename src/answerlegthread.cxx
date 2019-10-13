#pragma implementation
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
#include <stdexcept>
#include <cstring>
#include "asn.hpp"
#include "q931.hpp"
#include "callcontrol.hpp"
#include "legthread.hpp"
#include "answerlegthread.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>
#include "rtpstat.hpp"
#include "session.hpp"
#include "rtpsession.hpp"
#include "h323call.hpp"
#include "dtmfrelay.hpp"

AnswerLegThread :: ~AnswerLegThread ( ) { }

AnswerLegThread :: AnswerLegThread ( H323Call * c, LegThread * * p, unsigned i, CommonCallDetails & com ) :
	LegThread ( c, p, i ), common ( com ), endInited ( false ) { }

void AnswerLegThread :: Main ( ) {
	init ( );
	while ( ! ended ( ) ) {
		if ( conf -> shuttingDown ( ) )
			break;
		if ( ! iteration ( ) )
			break;
		if ( int t = common.getSigOptions ( ).maxCallDurationMin ( ) )
			if ( t <= ( PTime ( ) - begTime ).GetMinutes ( ) ) {
				PSYSTEMLOG ( Error, "max call duration: " << t << ", " << PTime ( ) << ", "
					<< begTime );
				common.setDisconnectCauseWeak ( Q931 :: cvNormalUnspecified );
				break;
			}
	}
	shutDown ( );
	call -> stop ( this, common.getDisconnectCause ( ) );
	common.release ( );
}

bool AnswerLegThread :: admissibleIP ( PIPSocket :: Address & ip ) {
	PSYSTEMLOG ( Info, "rtp ip check: " << ip );
	return conf -> validInRtp ( common.getSource ( ).peer, static_cast < const char * > ( ip.AsString ( ) ) );
}

void AnswerLegThread :: closeLocal ( int cause ) {
	common.setDisconnectCause ( Q931 :: CauseValues ( cause ) );
	PSYSTEMLOG(Info, "AnswerLegThread :: closeLocal: " << common.getDisconnectCause ( ) );
	shutDown ( );
}

void AnswerLegThread :: Close ( int cause ) {
	putMessage ( std :: tr1 :: bind ( & AnswerLegThread :: closeLocal, this, cause ) );
}

void AnswerLegThread :: released ( Q931 :: CauseValues /*cause*/ ) {
	* ( int * ) 0 = 1;
}

void AnswerLegThread :: peerAlertedLocal ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & outCodec ) {
	peerAlertedLeg ( localRtpPort, localAddr, inCodec, outCodec );
}

void AnswerLegThread :: peerAlerted ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & outCodec ) {
	putMessage ( std :: tr1 :: bind ( & AnswerLegThread :: peerAlertedLocal, this, localRtpPort, localAddr, inCodec,
		outCodec ) );
}

void AnswerLegThread :: peerConnectedLocal ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & outCodec, unsigned telephoneEventsPayloadType ) {
	begTime = PTime ( );
	begInited = true;
	if ( telephoneEventsPayloadType )
		common.setTelephoneEventsPayloadType ( telephoneEventsPayloadType );
	//conf -> addCall ( this ); -- nado chto-to delat. ili conf sam vse delaet ...
	peerConnectedLeg ( localRtpPort, localAddr, inCodec, outCodec );
}

void AnswerLegThread :: peerConnected ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & outCodec, unsigned telephoneEventsPayloadType ) {
	putMessage ( std :: tr1 :: bind ( & AnswerLegThread :: peerConnectedLocal, this, localRtpPort, localAddr, inCodec,
		outCodec, telephoneEventsPayloadType ) );
}

void AnswerLegThread :: shutDown ( int cause ) {
	if ( ! endInited ) {
		endInited = true;
		endTime = PTime ( );
		if ( cause )
			call -> released ( Q931 :: CauseValues ( cause ) );
		if ( begInited )
			common.setDisconnectCauseWeak ( cause ? : Q931 :: cvNormalCallClearing );
		else {
			common.setDisconnectCauseWeak ( cause ? : Q931 :: cvNormalUnspecified );
			begTime = PTime ( );
			begInited = true;
		}
		shutDownLeg ( );
	}
}

void AnswerLegThread :: peerOnHoldLocal ( int level ) {
	peerOnHoldLeg ( level );
}

void AnswerLegThread :: peerOnHold ( int level ) {
	putMessage ( std :: tr1 :: bind ( & AnswerLegThread :: peerOnHoldLocal, this, level ) );
}

void AnswerLegThread :: peerOnHoldOKLocal ( ) {
	peerOnHoldOKLeg ( );
}

void AnswerLegThread :: peerOnHoldOK ( ) {
	putMessage ( std :: tr1 :: bind ( & AnswerLegThread :: peerOnHoldOKLocal, this ) );
}

void AnswerLegThread :: peerDtmf ( const DTMF :: Relay & r ) {
	putMessage ( std :: tr1 :: bind ( & AnswerLegThread :: peerDtmfLocal, this, r ) );
}

void AnswerLegThread :: peerDtmfLocal ( const DTMF :: Relay & r ) {
	peerDtmfLeg ( r );
}

