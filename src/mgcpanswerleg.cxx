#pragma implementation
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
#include "mgcpepthreadhandler.hpp"
#include "mgcpanswerleg.hpp"
#include <queue>
#include "condvar.hpp"
#include "automutex.hpp"
#include "threadmessagequeue.hpp"
#include "mgcpthread.hpp"
#include "rtpstat.hpp"
#include "session.hpp"
#include "ipport.hpp"
#include "rtpsession.hpp"
#include "h323call.hpp"

struct MgcpAnswerLegThread :: Data {
	ThreadMessageQueue < PeerMessage > q;
};

void MgcpAnswerLegThread :: putMessage ( const PeerMessage & m ) {
	p -> q.put ( m );
}

void MgcpAnswerLegThread :: detach ( ) {
	p -> q.put ( std :: tr1 :: bind ( & MgcpAnswerLegThread :: detachLocal, this ) );
}

void MgcpAnswerLegThread :: detachLocal ( ) {
	detached = true;
	common.setDisconnectCauseWeak ( Q931 :: cvNormalCallClearing );
	call -> released ( Q931 :: cvNormalCallClearing );
}

bool MgcpAnswerLegThread :: iteration ( ) {
	PeerMessage m;
	if ( p -> q.get ( m ) )
		m ( );
	return true;
}

bool MgcpAnswerLegThread :: ended ( ) const {
	return detached;
}

void MgcpAnswerLegThread :: shutDownLeg ( ) {
	if ( ! detached ) {
		mgcpThread -> detachThreadHandler ( gw, ep, common.getDisconnectCause ( ) == Q931 :: cvNormalCallClearing );
		detached = true;
	}
}

void MgcpAnswerLegThread :: init ( ) { }

void MgcpAnswerLegThread :: peerAlertedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & /*outCodec*/ ) {
	mgcpThread -> sendRinging ( gw, ep, localRtpPort, localAddr, inCodec );
}

void MgcpAnswerLegThread :: peerConnectedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & /*outCodec*/ ) {
	mgcpThread -> sendOk ( gw, ep, localRtpPort, localAddr, inCodec, common.getTelephoneEventsPayloadType ( ) );
}

MgcpAnswerLegThread :: MgcpAnswerLegThread ( H323Call * c, LegThread * * p, unsigned i, const CommonCallDetails & d,
	const ss :: string & g, const ss :: string & e ) : AnswerLegThread ( c, p, i, ccd ), ccd ( d ), gw ( g ), ep ( e ),
	p ( new Data ), detached ( false ) {
	Resume ( );
}

MgcpAnswerLegThread :: ~MgcpAnswerLegThread ( ) {
	delete p;
}
