#pragma implementation
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "ss.hpp"
#include "allocatable.hpp"
#include "ixcudpsocket.hpp"
#include "SipCallThread.h"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include "sip.hpp"
#include "codecinfo.hpp"
#include "rtpstat.hpp"
#include "session.hpp"
#include "rtpsession.hpp"
#include "callcontrol.hpp"
#include "pointer.hpp"
#include "signallingoptions.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "sourcedata.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include "condvar.hpp"
#include "SIPAuthenticateValidator.hpp"
#include <queue>
#include "automutex.hpp"
#include "threadmessagequeue.hpp"
#include "sipsignallingthread.hpp"
#include "ssconf.hpp"
#include <ptlib/svcproc.h>
SipCallThread :: SipCallThread ( ) :
	PThread ( 1000, NoAutoDeleteThread, NormalPriority, "SipCallThread" ) {
	// Make sure you open the socket before calling Resume
	PSYSTEMLOG ( Info, "SipCall Thread :: Constructor" );
//	Sleep ( 30000 );
	while ( ! CallSocket.Listen ( PIPSocket :: Address ( "*" ), 5,
		5060, PSocket :: CanReuseAddress ) ) {
		PSYSTEMLOG ( Error, "Listen failed on call signalling socket: " <<
			CallSocket.GetErrorText ( ) );
		if ( conf -> shuttingDown ( ) )
			break;
		Sleep ( 1000 );
	}
	Resume ( );
}

SipCallThread :: ~SipCallThread ( ) { }

void SipCallThread :: Main ( ) {
	if ( disableSIP )
		return;
	PSYSTEMLOG ( Info, "SipCall Thread :: Main begin" );
	unsigned sId = 0;
	CallSocket.SetReadTimeout ( 10000 );
	SipSignallingThread * t =
		new SipSignallingThread ( & CallSocket, sId );
	PSYSTEMLOG ( Info, "SipCall Thread : signalling thread started" );
	while ( CallSocket.IsOpen ( ) )
		Sleep ( 10000 );
	t -> Close ( );
	t -> WaitForTermination ( );
	delete t;
	PSYSTEMLOG ( Info, "SipCall thread ended" );
}

void SipCallThread :: Close ( ) {
	CallSocket.Close ( );
}
