#pragma implementation
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "CallThread.h"
#include "ss.hpp"
#include "allocatable.hpp"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "callcontrol.hpp"
#include <stdexcept>
#include <limits>
#include <cstring>
#include "asn.hpp"
#include "h225.hpp"
#include "latencylimits.hpp"
#include "fakecalldetector.hpp"
#include "rtpstat.hpp"
#include "signallingoptions.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "sourcedata.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "pointer.hpp"
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include <boost/multi_index/member.hpp>
#include "q931.hpp"
#include "calldetails.hpp"
#include "signallingthread.hpp"
#include <ptlib/svcproc.h>
#include "automutex.hpp"
#include "ssconf.hpp"

CallThread :: CallThread ( ) :
	PThread ( 1000, NoAutoDeleteThread, NormalPriority, "CallThread" ) {
	// Make sure you open the socket before calling Resume
	while ( ! callSocket.Listen ( PIPSocket :: Address ( "*" ), 5,
		STD_SIGNALLING_PORT, PSocket :: CanReuseAddress ) ) {
		PSYSTEMLOG ( Error, "Listen failed on call signalling socket: " <<
			callSocket.GetErrorText ( ) );
		if ( conf -> shuttingDown ( ) )
			break;
		Sleep ( 1000 );
	}
	Resume ( );
}

CallThread :: ~CallThread ( ) { }

int getNewCallId ( ) {
	static unsigned id = 0;
	return __sync_add_and_fetch ( & id, 1 );
}

void CallThread :: Main ( ) {
	PSYSTEMLOG ( Info, "Call Thread :: Main begin" );
	callSocket.SetReadTimeout ( 10000 );
	while ( callSocket.IsOpen ( ) ) {
		if ( conf -> shuttingDown ( ) )
			break;
		PTCPSocket * callChannel = new PTCPSocket;
		if ( callChannel -> Accept ( callSocket ) ) {
			PSYSTEMLOG ( Info, "New incoming connection to call signaling port" );
			// We have a new call, spawn a thread to handle it
			// It will handle the clean up of the socket when it finishes...
			if ( conf -> tryTake ( ) ) {
				new SignallingThread ( callChannel, getNewCallId ( ) );
				continue;
			}
		} else if ( callChannel -> GetErrorCode ( ) != PChannel :: Timeout &&
			callChannel -> GetErrorCode ( ) != PChannel :: Interrupted )
			PSYSTEMLOG ( Error, "Error in accepting call: " << callChannel -> GetErrorText ( ) );
		delete callChannel;
	}
	PSYSTEMLOG ( Info, "Call thread ended" );
}

void CallThread :: Close ( ) {
	callSocket.Close ( );
}
