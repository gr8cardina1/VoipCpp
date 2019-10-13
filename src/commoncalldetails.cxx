#pragma implementation
#pragma implementation "smartguard.hpp"

#include "ss.hpp"
#include "allocatable.hpp"
#include "tarifinfo.hpp"
#include "tarifround.hpp"
#include "pricedata.hpp"
#include "sourcedata.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include "signallingoptions.hpp"
#include "outcarddetails.hpp"
#include "pointer.hpp"
#include "aftertask.hpp"
#include <ptlib.h>
#include "moneyspool.hpp"
#include "outchoicedetails.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include "backtrace.hpp"
#include <ptlib/sockets.h>
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>
#include "q931.hpp"

void CommonCallDetails :: release ( ) const {
	conf -> release ( inTaken ? _source.peer : 0 );
	releaseGuard -> dismiss ( );
}

class DestructLog {
	PTime createdTime;
	TraceVector createdTrace;
public:
//	typedef void result_type;
	DestructLog ( ) : createdTrace ( getBacktrace ( ) ) { }
	void operator ( ) ( ) {
		ss :: ostringstream os;
		os << "object destroyed " << PTime ( ).AsString ( ) << '\n';
		printBacktrace ( os );
		os << "created " << createdTime.AsString ( ) << '\n';
		printBacktrace ( os, createdTrace );
		PSYSTEMLOG ( Error, os.str ( ) );
	}
};

CommonCallDetails :: CommonCallDetails ( ) : releaseGuard ( new SmartGuard ( DestructLog ( ) ) ), callerPort ( 0 ),
	fromNat ( 0 ), disconnectCause ( conf -> getDefaultDisconnectCause()/*Q931 :: cvNoCircuitChannelAvailable*/ ), peerIndex ( - 1 ),
	uTelephoneEventsPayloadType_(0), _inAllCodecs ( false ), _disconnectCauseInited ( false ),
	forceDebug ( false ), isGk ( false ), outRemotePrice ( false ), outAllCodecs ( false ),
	isAnonymous_ ( false ), inTaken ( false ), directIn ( false ) { }

void CommonCallDetails :: dropNonH323 ( ) {
	choices.erase ( std :: remove_if ( choices.begin ( ), choices.end ( ),
		boost :: bind ( & OutChoiceDetails :: getType, _1 ) != ptH323 ), choices.end ( ) );
}

void CommonCallDetails :: dropNonSIP ( ) {
	choices.erase ( std :: remove_if ( choices.begin ( ), choices.end ( ),
		boost :: bind ( & OutChoiceDetails :: getType, _1 ) != ptSip ), choices.end ( ) );
}

