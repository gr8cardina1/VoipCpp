#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
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
#include "aftertask.hpp"
#include <ptlib.h>
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include "radiuscommon.hpp"
#include "radius.hpp"
#include "rtpstat.hpp"
#include <ptlib/sockets.h>
#include "Log.h"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>
#include "ssconf.hpp"

void sendRadiusAcc ( const CommonCallDetails & common, const RTPStat * rtpStats, PTime setupTime, PTime outSetupTime,
	PTime begTime, PTime endTime, int ref, const ss :: string & incomingCallId ) {
	ss :: string radiusIp = conf -> getRadiusIp ( );
	if ( radiusIp.empty ( ) )
		return;
	int radiusPort = conf -> getRadiusPort ( );
	bool hasCurChoice = common.curChoice ( );
	using namespace Radius;
	ss :: string radiusSecret = conf -> getRadiusSecret ( );
	Request r ( radiusSecret, rAccountingRequest, radiusIp, radiusPort + 1 );
	Request r2 ( radiusSecret, rAccountingRequest, radiusIp, radiusPort + 1 );
	Request r3 ( radiusSecret, rAccountingRequest, radiusIp, radiusPort + 1 );
	Request r4 ( radiusSecret, rAccountingRequest, radiusIp, radiusPort + 1 );
	PIPSocket :: Address nas;
	PIPSocket :: GetHostAddress ( nas );
	Pointer < Attribute > p = new NASIPAddress ( htonl ( nas ) );
	r.append ( p );
	p = new NASIPAddress ( htonl ( nas ) );
	r2.append ( p );
	p = new NASIPAddress ( htonl ( nas ) );
	r3.append ( p );
	p = new NASIPAddress ( htonl ( nas ) );
	r4.append ( p );
	p = new CiscoNASPort ( "" );
	r.append ( p );
	p = new CiscoNASPort ( "" );
	r2.append ( p );
	p = new CiscoNASPort ( "" );
	r3.append ( p );
	p = new CiscoNASPort ( "" );
	r4.append ( p );
	p = new NASPortType ( npVirtual );
	r.append ( p );
	p = new NASPortType ( npVirtual );
	r2.append ( p );
	p = new NASPortType ( npVirtual );
	r3.append ( p );
	p = new NASPortType ( npVirtual );
	r4.append ( p );
	ss :: ostringstream os;
	if ( hasCurChoice )
		os << common.curChoice ( ) -> getPeer ( );
	p = new UserName ( os.str ( ) );
	r.append ( p );
	p = new UserName ( os.str ( ) );
	r3.append ( p );
	p = new H323RemoteId ( os.str ( ) );
	r.append ( p );
	p = new H323RemoteId ( os.str ( ) );
	r3.append ( p );
	os.str ( "" );
	os << common.getSource ( ).peer;
	p = new UserName ( os.str ( ) );
	r2.append ( p );
	p = new UserName ( os.str ( ) );
	r4.append ( p );
	p = new H323RemoteId ( os.str ( ) );
	r2.append ( p );
	p = new H323RemoteId ( os.str ( ) );
	r4.append ( p );
	if ( hasCurChoice ) {
		p = new CalledStationId ( normalisedRadiusAcc ? common.getRealDigits ( ) : common.getConvertedDigits ( ) );
		r.append ( p );
		p = new CalledStationId ( normalisedRadiusAcc ? common.getRealDigits ( ) : common.getConvertedDigits ( ) );
		r3.append ( p );
		p = new H323RemoteAddress ( common.getCalledIp ( ) );
		r.append ( p );
		p = new H323RemoteAddress ( common.getCalledIp ( ) );
		r3.append ( p );
		p = new CallingStationId ( common.getCallingDigits ( ) );
		r.append ( p );
		p = new CallingStationId ( common.getCallingDigits ( ) );
		r3.append ( p );
	} else {
		p = new CallingStationId ( common.getCallingDigitsIn ( ) );
		r.append ( p );
		p = new CallingStationId ( common.getCallingDigitsIn ( ) );
		r3.append ( p );
	}
	p = new CalledStationId ( normalisedRadiusAcc ? common.getRealDigits ( ) : common.getDialedDigits ( ) );
	r2.append ( p );
	p = new CalledStationId ( normalisedRadiusAcc ? common.getRealDigits ( ) : common.getDialedDigits ( ) );
	r4.append ( p );
	p = new H323RemoteAddress ( common.getCallerIp ( ) );
	r2.append ( p );
	p = new H323RemoteAddress ( common.getCallerIp ( ) );
	r4.append ( p );
	p = new CallingStationId ( common.getCallingDigitsIn ( ) );
	r2.append ( p );
	p = new CallingStationId ( common.getCallingDigitsIn ( ) );
	r4.append ( p );
	p = new AcctStatusType ( sStop );
	r.append ( p );
	p = new AcctStatusType ( sStop );
	r2.append ( p );
	p = new AcctStatusType ( sStart );
	r3.append ( p );
	p = new AcctStatusType ( sStart );
	r4.append ( p );
	p = new ServiceType ( sLogin );
	r.append ( p );
	p = new ServiceType ( sLogin );
	r2.append ( p );
	p = new ServiceType ( sLogin );
	r3.append ( p );
	p = new ServiceType ( sLogin );
	r4.append ( p );
	const ss :: string & name = conf -> getName ( );
	p = new H323GWId ( name );
	r.append ( p );
	p = new H323GWId ( name );
	r2.append ( p );
	p = new H323GWId ( name );
	r3.append ( p );
	p = new H323GWId ( name );
	r4.append ( p );
	const ss :: string & confId = common.getConfId ( );
	if ( confId.size ( ) != 16 )
		return;
	os.str ( "" );
	typedef unsigned char uchar;
	int a = uchar ( confId [ 0 ] ) * 0x1000000 + uchar ( confId [ 1 ] ) * 0x10000 + uchar ( confId [ 2 ] ) * 0x100 + uchar ( confId [ 3 ] );
	os << std :: hex << a << ' ';
	a = uchar ( confId [ 4 ] ) * 0x1000000 + uchar ( confId [ 5 ] ) * 0x10000 + uchar ( confId [ 6 ] ) * 0x100 + uchar ( confId [ 7 ] );
	os << std :: hex << a << ' ';
	a = uchar ( confId [ 8 ] ) * 0x1000000 + uchar ( confId [ 9 ] ) * 0x10000 + uchar ( confId [ 10 ] ) * 0x100 + uchar ( confId [ 11 ] );
	os << std :: hex << a << ' ';
	a = uchar ( confId [ 12 ] ) * 0x1000000 + uchar ( confId [ 13 ] ) * 0x10000 + uchar ( confId [ 14 ] ) * 0x100 + uchar ( confId [ 15 ] );
	os << std :: hex << a;
	p = new H323ConfId ( os.str ( ) );
	r.append ( p );
	p = new H323ConfId ( incomingCallId.empty ( ) ? os.str ( ) : incomingCallId );
	r2.append ( p );
	p = new H323ConfId ( os.str ( ) );
	r3.append ( p );
	p = new H323ConfId ( incomingCallId.empty ( ) ? os.str ( ) : incomingCallId );
	r4.append ( p );
	p = new H323CallOrigin ( "originate" );
	r.append ( p );
	p = new H323CallOrigin ( "answer" );
	r2.append ( p );
	p = new H323CallOrigin ( "originate" );
	r3.append ( p );
	p = new H323CallOrigin ( "answer" );
	r4.append ( p );
	p = new H323CallType ( "VoIP" );
	r.append ( p );
	p = new H323CallType ( "VoIP" );
	r2.append ( p );
	p = new H323CallType ( "VoIP" );
	r3.append ( p );
	p = new H323CallType ( "VoIP" );
	r4.append ( p );
	static const char * timeFormat = "hh:mm:ss.666 z www MMM d yyyy";
	p = new H323SetupTime ( static_cast < const char * > ( outSetupTime.AsString ( timeFormat ) ) );
	r.append ( p );
	p = new H323SetupTime ( static_cast < const char * > ( setupTime.AsString ( timeFormat ) ) );
	r2.append ( p );
	p = new H323SetupTime ( static_cast < const char * > ( outSetupTime.AsString ( timeFormat ) ) );
	r3.append ( p );
	p = new H323SetupTime ( static_cast < const char * > ( setupTime.AsString ( timeFormat ) ) );
	r4.append ( p );
	p = new H323ConnectTime ( static_cast < const char * > ( begTime.AsString ( timeFormat ) ) );
	r.append ( p );
	p = new H323ConnectTime ( static_cast < const char * > ( begTime.AsString ( timeFormat ) ) );
	r2.append ( p );
	p = new H323ConnectTime ( static_cast < const char * > ( begTime.AsString ( timeFormat ) ) );
	r3.append ( p );
	p = new H323ConnectTime ( static_cast < const char * > ( begTime.AsString ( timeFormat ) ) );
	r4.append ( p );
	p = new H323DisconnectTime ( static_cast < const char * > ( endTime.AsString ( timeFormat ) ) );
	r.append ( p );
	p = new H323DisconnectTime ( static_cast < const char * > ( endTime.AsString ( timeFormat ) ) );
	r2.append ( p );
	os.str ( "" );
	os << std :: hex << common.getDisconnectCause ( );
	p = new H323DisconnectCause ( os.str ( ) );
	r.append ( p );
	p = new H323DisconnectCause ( os.str ( ) );
	r2.append ( p );
	p = new H323VoiceQuality ( "0" );
	r.append ( p );
	p = new H323VoiceQuality ( "0" );
	r2.append ( p );
	os.str ( "" );
	os << std :: hex << ref;
	p = new AcctSessionId ( os.str ( ) );
	r.append ( p );
	p = new AcctSessionId ( os.str ( ) );
	r2.append ( p );
	p = new AcctSessionId ( os.str ( ) );
	r3.append ( p );
	p = new AcctSessionId ( os.str ( ) );
	r4.append ( p );
	p = new AcctSessionTime ( int ( ( endTime - begTime ).GetSeconds ( ) ) );
	r.append ( p );
	p = new AcctSessionTime ( int ( ( endTime - begTime ).GetSeconds ( ) ) );
	r2.append ( p );
	p = new AcctInputOctets ( rtpStats [ 2 ].bytesCount );
	r.append ( p );
	p = new AcctInputOctets ( rtpStats [ 3 ].bytesCount );
	r2.append ( p );
	p = new AcctOutputOctets ( rtpStats [ 0 ].bytesCount );
	r.append ( p );
	p = new AcctOutputOctets ( rtpStats [ 1 ].bytesCount );
	r2.append ( p );
	p = new AcctInputPackets ( rtpStats [ 2 ].packetCount );
	r.append ( p );
	p = new AcctInputPackets ( rtpStats [ 3 ].packetCount );
	r2.append ( p );
	p = new AcctOutputPackets ( rtpStats [ 0 ].packetCount );
	r.append ( p );
	p = new AcctOutputPackets ( rtpStats [ 1 ].packetCount );
	r2.append ( p );
	PUDPSocket sock;
	sock.SetSendAddress ( PIPSocket :: Address ( radiusIp.c_str ( ) ), WORD ( radiusPort + 1 ) );
	char buf [ 2000 ];
	radiusLog -> logRadiusMsg ( r3, OpengateLog :: Sending );
	int l = r3.print ( buf, 2000 );
	sock.Write ( buf, l );
	radiusLog -> logRadiusMsg ( r4, OpengateLog :: Sending );
	l = r4.print ( buf, 2000 );
	sock.Write ( buf, l );
	radiusLog -> logRadiusMsg ( r, OpengateLog :: Sending );
	l = r.print ( buf, 2000 );
	sock.Write ( buf, l );
	radiusLog -> logRadiusMsg ( r2, OpengateLog :: Sending );
	l = r2.print ( buf, 2000 );
	sock.Write ( buf, l );
}

void externalRoute ( int timeout, const ss :: string & secret, const ss :: string & ani, int inPeer,
	const ss :: string & dialedDigits, IntVector & outPeers, ss :: string & replaceAni ) {
	ss :: string radiusIp = conf -> getRadiusIp ( );
	if ( radiusIp.empty ( ) )
		return;
	int radiusPort = conf -> getRadiusPort ( );
	if ( timeout < 1 )
		timeout = 1;
	else if ( timeout > 15 )
		timeout = 15;
	using namespace Radius;
	Pointer < Request > r = new Request ( secret, rAccessRequest, radiusIp, radiusPort );
	Pointer < Attribute > p = new H323IvrOut ( "ANI:" + ani );
	r -> append ( p );
	ss :: ostringstream os;
	os << "Inbound:" << inPeer;
	p = new H323IvrOut ( os.str ( ) );
	r -> append ( p );
	p = new H323IvrOut ( "Dialed:" + dialedDigits );
	r -> append ( p );
	radiusLog -> logRadiusMsg ( * r, OpengateLog :: Sending );
	unsigned char buf [ 2000 ];
	int l = r -> print ( reinterpret_cast < char * > ( buf ), 2000 );
	PUDPSocket sock;
	sock.SetSendAddress ( PIPSocket :: Address ( radiusIp.c_str ( ) ), WORD ( radiusPort ) );
	sock.Write ( buf, l );
	sock.SetReadTimeout ( timeout * 1000 );
	if ( ! sock.Read ( buf, 2000 ) ) {
		PSYSTEMLOG ( Error, "cant read radius socket(" << inPeer << ',' << ani << ',' << dialedDigits << "): "
			<< sock.GetErrorText ( ) );
		return;
	}
	PIPSocket :: Address addr;
	WORD port;
	sock.GetLastReceiveAddress ( addr, port );
	try {
		r = new Request ( secret, static_cast < const char * > ( addr.AsString ( ) ), port, buf,
			sock.GetLastReadCount ( ) );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "externalRoute exception: " << e.what ( ) );
		return;
	}
	radiusLog -> logRadiusMsg ( * r, OpengateLog :: Receiving );
	if ( r -> getCode ( ) != rAccessAccept ) {
		PSYSTEMLOG ( Error, "got " << r -> getCodeName ( ) );
		return;
	}
	if ( const AttributeCisco * replace = r -> findCisco ( cH323Ivr, "h323-ivr-in=ReplaceANI:" ) )
		replaceAni = replace -> getVal ( ).substr ( 23 );
	int skip = 0;
	while ( const AttributeCisco * out = r -> findCisco ( cH323Ivr, "h323-ivr-in=Outbound:", skip ++ ) ) {
		int id = std :: atoi ( out -> getVal ( ).substr ( 21 ).c_str ( ) );
		PSYSTEMLOG ( Info, "returned route: " << id );
		outPeers.push_back ( id );
	}
	return;
}

