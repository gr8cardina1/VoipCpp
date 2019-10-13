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
#include "q931.hpp"
#include "callcontrol.hpp"
#include "legthread.hpp"
#include <queue>
#include "originatelegthread.hpp"
#include "mgcpepthreadhandler.hpp"
#include "mgcpleg.hpp"
#include "condvar.hpp"
#include "automutex.hpp"
#include "threadmessagequeue.hpp"
#include "mgcpthread.hpp"
#include <ptlib/svcproc.h>
#include <boost/range.hpp>
#include "slprint.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include "ixcudpsocket.hpp"
#include "sdp.hpp"
#include "sdpcommon.hpp"


struct MgcpLeg :: Data {
public:
	typedef std :: tr1 :: function < bool ( ) > MgcpMessage;
	ThreadMessageQueue < MgcpMessage > q;
};

MgcpLeg :: MgcpLeg ( OriginateLegThread * t, CommonCallDetails & c, const RecodeInfoVector & inCodecs,
	const ss :: string & g, const ss :: string & e ) : Leg ( t, c ), inRecodes ( inCodecs ), gw ( g ), ep ( e ),
	p ( new Data ), detached ( true ) { }

MgcpLeg :: ~MgcpLeg ( ) {
	if ( ! detached )
		mgcpThread -> detachThreadHandler ( gw, ep, true );
	delete p;
}

namespace H323 {
ss :: string globallyUniqueId ( );
ss :: string printableCallId ( const ss :: string & confId );
}

bool MgcpLeg :: tryChoice ( ) {
	PIPSocket :: Address localAddr = thread -> getLocalIp ( );
	int rtpPort = thread -> getLocalPort ( );
	common.setConfId ( H323 :: globallyUniqueId ( ) );
	thread -> setPrintableCallId ( common.getConfId ( ) );
	if ( ! mgcpThread -> originateCall ( gw, ep, this, H323 :: printableCallId ( common.getConfId ( ) ), inRecodes.begin ( ) -> codec,
		common.getTelephoneEventsPayloadType ( ), rtpPort, localAddr ) )
		return false;
	detached = false;
	thread -> calledSocketConnected ( localAddr );
	return true;
}

bool MgcpLeg :: initChoice ( ) {
	return true;
}

void MgcpLeg :: sdpAck ( const SDP :: SessionDescription & sdp ) {
	p -> q.put ( std :: tr1 :: bind ( & MgcpLeg :: sdpAckLocal, this, sdp ) );
}

void MgcpLeg :: connected ( ) {
	p -> q.put ( std :: tr1 :: bind ( & MgcpLeg :: connectedLocal, this ) );
}

void MgcpLeg :: detach ( ) {
	p -> q.put ( std :: tr1 :: bind ( & MgcpLeg :: detachLocal, this ) );
}

bool MgcpLeg :: iteration ( ) {
	Data :: MgcpMessage m;
	if ( p -> q.get ( m ) )
		return m ( );
	return true;
}

void MgcpLeg :: closeChoice ( ) {
	if ( ! detached )
		mgcpThread -> detachThreadHandler ( gw, ep, true );
}

bool MgcpLeg :: ended ( ) const {
	return detached;
}

void MgcpLeg :: wakeUp ( ) {
	p -> q.wake ( );
}

bool MgcpLeg :: sdpAckLocal ( const SDP :: SessionDescription & sdp ) {
	thread -> answered ( );
	if ( const SDP :: MediaDescription * media = sdp.getMediaDescription ( SDP :: mediaAudio ) ) {
		if ( unsigned payload = getTelephoneEventsPayloadType ( media ) )
			common.setTelephoneEventsPayloadType ( payload );
		if ( unsigned payload = common.getTelephoneEventsPayloadType ( ) )
			thread -> setTelephoneEventsPayloadType ( payload );
		const SDP :: MediaFormatVector & formats = media -> getFmts ( );
		unsigned ptime = getPtime ( media );
		for ( SDP :: MediaFormatVector :: const_iterator i = formats.begin ( ); i != formats.end ( ); ++ i ) {
			CodecInfo codec = getCodec ( * i, ptime ), changedCodec = codec;
			typedef RecodeInfoVector :: index < Codec > :: type ByCodec;
			ByCodec & inByCodec = inRecodes.get < Codec > ( );
			ByCodec :: const_iterator i = inByCodec.find ( codec );
			if ( i == inByCodec.end ( ) ) {
				PSYSTEMLOG ( Error, "inByCodec not found: " << codec );
				ss :: for_each ( inByCodec, SLPrint ( "recode: " ) );
				continue;
			} else
				codec = i -> backCodec;
			PIPSocket :: Address addr;
			if ( const ss :: string * ip = media -> getConnectionAddress ( ) )
				addr = ip -> c_str ( );
			else
				addr = sdp.getConnectionAddress ( ).c_str ( );
			thread -> setCodec ( codec );
			thread -> setSendAddress ( addr, media -> getPort ( ), codec, codec, changedCodec, changedCodec );
			thread -> alerted ( );
			return true;
		}
	}
	thread -> alerted ( );
	return false;
}

bool MgcpLeg :: connectedLocal ( ) {
	thread -> connected ( );
	return true;
}

bool MgcpLeg :: detachLocal ( ) {
	detached = true;
	thread -> released ( Q931 :: cvNormalCallClearing );
	return false;
}
