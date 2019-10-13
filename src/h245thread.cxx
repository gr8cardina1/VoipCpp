#pragma implementation
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "ss.hpp"
#include "allocatable.hpp"
#include "h245thread.hpp"
#include "Log.h"
#include <stdexcept>
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "sourcedata.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include <cstring>
#include "asn.hpp"
#include "q931.hpp"
#include "h225.hpp"
#include "h323common.hpp"
#include "h245handler.hpp"
#include "AddrUtils.h"
#include <ptlib/svcproc.h>
#include "automutex.hpp"
#include "pointer.hpp" //safeDel

H245Thread :: ~H245Thread ( ) {
	PSYSTEMLOG ( Info, "~H245Thread ( )" );
	safeDel ( callerSocket );
	safeDel ( calledSocket );
}

#define LOGINFO( variables ) do { if ( forceDebug ) { PSYSTEMLOG ( Error, variables ); } else PSYSTEMLOG ( Info, variables ); } while ( 0 )

H245Thread :: H245Thread ( const H225 :: TransportAddress & calledAddress, H245Handler * _h, int ref, bool fd, bool bc,
	bool l2 ) : PThread ( 1000, NoAutoDeleteThread, NormalPriority, "H245 " + PString ( ref ) ),
	sessionEnded ( false ), h ( _h ), forceDebug ( fd ), byCaller ( bc ), listening ( true ), needConnect ( false ) {
	WORD port;
	if ( ! l2 ) {
		PAssert ( ( calledAddress.getTag ( ) == H225 :: TransportAddress :: e_ipAddress ),
			"H245 target address is not an IP address" );
		const H225 :: TransportAddress_ipAddress & calledIP = calledAddress;
		AddrUtils :: convertToIPAddress ( calledIP, calledAddr, port );
		if ( port == 0 )
			PSYSTEMLOG ( Error, "can't convert address: " << calledIP << "!!!!!!!!!!!!!!!!!!" );
	}
	calledSocket = new PTCPSocket ( port );
	callerSocket = new PTCPSocket;
	calledSocket -> SetReadTimeout ( 10000 );
	callerSocket -> SetReadTimeout ( 10000 );
	calledSocket -> SetWriteTimeout ( 10000 );
	callerSocket -> SetWriteTimeout ( 10000 );
	listener.SetReadTimeout ( 30000 );
	listener2.SetReadTimeout ( 30000 );
	// Make sure you open the socket before calling Resume
	if ( ! listener.Listen ( 1 ) )
		PSYSTEMLOG ( Error, "Listen failed on H245 socket: " <<
			listener.GetErrorText ( ) );
	else {
		if ( ! listener.GetLocalAddress ( origAddr, origPort ) )
			PSYSTEMLOG ( Error, "first getListenerPort: " << listener.GetErrorText ( ) );
		LOGINFO ( "Listen succeded on H245 socket on port " << getListenerPort ( ) );
	}
	if ( l2 && ! listener2.Listen ( 1 ) )
		PSYSTEMLOG ( Error, "Listen2 failed on H245 socket: " << listener2.GetErrorText ( ) );
	Resume ( );
}
bool H245Thread :: getBC ( ) {
	return byCaller;
}
WORD H245Thread :: getListenerPort ( )
// Task: to return the port of the listener socket to give to the caller.
// This metod should replace the GetAddress() metod for multihomed enviroment handling.
{
	PIPSocket :: Address Addr;
	WORD Port;
	if ( listener.GetLocalAddress ( Addr, Port ) ) {
		if ( Addr != origAddr || Port != origPort )
			PSYSTEMLOG ( Error, "getListenerPort: got " << Addr << ':' << Port << ", orig: "
				<< origAddr << ':' << origPort );
		return Port;
	}
	PSYSTEMLOG ( Error, "getListenerPort: " << listener.GetErrorText ( ) );
	return 0;
}

WORD H245Thread :: getListenerPort2 ( )
// Task: to return the port of the listener socket to give to the caller.
// This metod should replace the GetAddress() metod for multihomed enviroment handling.
{
	PIPSocket :: Address Addr;
	WORD Port;
	if ( listener2.GetLocalAddress ( Addr, Port ) )
		return Port;
	PSYSTEMLOG ( Error, "getListenerPort2: " << listener2.GetErrorText ( ) );
	return 0;
}

void H245Thread :: abortListen ( const PIPSocket :: Address & addr, WORD port ) {
	AutoMutex am ( mut );
	if ( needConnect )
		return;
	if ( ! listening )
		return;
	connectAddr = addr;
	connectPort = port;
	needConnect = true;
	PXAbortBlock ( );
}

bool H245Thread :: connectCaller ( ) {
#if PTRACING
	WORD listenerPort = getListenerPort ( );
#endif
	PTime beg;
	bool r = callerSocket -> Accept ( listener );
	{
		AutoMutex am ( mut );
		listening = false;
		if ( r )
			return true;
	}
	if ( ! listener.IsOpen ( ) ) {
		PSYSTEMLOG ( Error, "H245Thread closed while connecting" );
		return false;
	}
	if ( ! needConnect ) {
		PSYSTEMLOG ( Error, "H245 Accept failed since " << beg << ": " <<
			listener.GetErrorText ( ) );
		PIPSocket :: Address addr;
		WORD port;
		if ( listener.GetLocalAddress ( addr, port ) )
			PSYSTEMLOG ( Error, "on " << addr << ':' << port << " ( " << listenerPort << " )" );
		else
			PSYSTEMLOG ( Error, "can't get local address for " << listenerPort << ": " << listener.GetErrorText ( ) );
		return false;
	}
	callerSocket -> SetPort ( connectPort );
	if ( callerSocket -> Connect ( ( byCaller ? h -> getTo ( ) : h -> getFrom ( ) ), connectAddr ) )
		return true;
	PSYSTEMLOG ( Error, "H245 accept aborted and connect to " << connectAddr << ':' <<
		connectPort << " failed: " << callerSocket -> GetErrorText ( ) );
	return false;
}

bool H245Thread :: connectionEstablished ( )
// Task: to wait for a connect from the caller on the listener socket, when it happens
// establish a connection to the called party...
// Returns true if successful
{
	if ( ! listener.IsOpen ( ) ) {
		PSYSTEMLOG ( Error, "H425 listener isn't open" );
		return false;
	}
	if ( ! connectCaller ( ) )
		return false;
	if ( listener2.IsOpen ( ) ) {
		if ( calledSocket -> Accept ( listener2 ) )
			return true;
		PSYSTEMLOG ( Error, "Accept2 failed: " << calledSocket -> GetErrorText ( ) );
		return false;
	}
	// Connect to the called
	if ( calledSocket -> GetPort ( ) == 0 ) {
		PSYSTEMLOG ( Error, "called port == 0 !!!!!!!!!!!!!!!!!!" );
		return false;
	}
	if ( calledSocket -> Connect ( ( byCaller ? h -> getFrom ( ) : h -> getTo ( ) ), calledAddr ) )
		return true;
	PSYSTEMLOG ( Error, "Failed to connect to H245 target " <<
		calledAddr << ", " << calledSocket -> GetErrorText ( ) );
	return false;
}

void H245Thread :: receiveMesg ( PTCPSocket * from, PTCPSocket * to ) {
// Task: to receive an H.245 message and handle it as appropriate
	// Put in log here...
	PSYSTEMLOG(Info, "H245Thread :: receiveMesg");

	BYTE tpkt [ 4 ];

	if ( ! from -> ReadBlock ( tpkt, sizeof ( tpkt ) ) ) {
		// Assume that the connection has been closed....
		PSYSTEMLOG(Info, "H245: Assume that the connection has been closed...");
		Close ( );
		return;
	}

	if ( tpkt [ 0 ] != 3 ) {
		PSYSTEMLOG ( Error, "Expecting TPKT in H.245" );
		Close ( );
		return;
	}
	int bufferSize = ( ( tpkt [ 2 ] << 8 ) | tpkt [ 3 ] ) - 4;
	Asn :: string buffer ( bufferSize, '\0' );
	if ( ! bufferSize || ! from -> Read ( & buffer [ 0 ], bufferSize ) ) {
		PSYSTEMLOG(Error, "Error of sockets data reading.");
		Close ( );
		return;
	}
	PSYSTEMLOG ( Info, "Received H245 message" );
	PIPSocket :: Address peerAddr;
	WORD peerPort;
	from -> GetPeerAddress ( peerAddr, peerPort );
	try {
		Asn :: istream is ( buffer );
		H245 :: MultimediaSystemControlMessage h245Mesg ( is );
		if ( is.hasException ( ) )
			PSYSTEMLOG ( Error, "decode exception: " << is.getException ( ) );
		Log -> logH245Msg ( h245Mesg, OpengateLog :: Receiving, peerAddr, peerPort, forceDebug );
		processMesg ( h245Mesg, ( from == callerSocket ) ^ byCaller, to, from );
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "decode exception: " << e.what ( ) );
	}
}

void H245Thread :: Main ( ) {
	PSYSTEMLOG ( Info, "H245Thread::Main() - begin" );
	if ( ! connectionEstablished ( ) ) {
		PSYSTEMLOG ( Error, "H245Thread::Main() - bad" );
		Close ( );
		return;
	}
	try {
		while ( ! sessionEnded && callerSocket -> IsOpen ( ) &&
			calledSocket -> IsOpen ( ) ) {
			int selection = PSocket :: Select ( * callerSocket, * calledSocket, 3000 );
			switch ( selection ) {
				case -1: // Data available on callerSocket
					receiveMesg ( callerSocket, calledSocket );
					break;
				case -2: // Data available on calledSocket
					receiveMesg ( calledSocket, callerSocket );
					break;
				case -3: // Data available on both
					receiveMesg ( callerSocket, calledSocket );
					receiveMesg ( calledSocket, callerSocket );
					break;
				case 0 : // Timeout
					break;
				default: // Error on select...
					PSYSTEMLOG ( Info, "Error " << selection << " ( " <<
						PChannel :: GetErrorText ( PChannel :: Errors ( selection ) ) <<
							" ) on H245 select" );
					break;
			}
		}
	} catch ( std :: logic_error & e ) {
		PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
	}
	Close ( );
	PSYSTEMLOG ( Info, "H245Thread::Main() - end" );
}
void H245Thread :: Close ( ) {
	sessionEnded = true;
	PSYSTEMLOG ( Info, "H245Thread::Close()" );
	listener.Close ( );
	callerSocket -> Close ( );
	calledSocket -> Close ( );
}

static void sendH245Mesg ( const H245 :: MultimediaSystemControlMessage & mesg, PTCPSocket * destination,
	bool forceDebug ) {
	Asn :: ostream os;
	os.initTpkt ( );
	mesg.encode ( os );
	os.finishTpkt ( );
	sendH245MesgToDest ( os.str ( ), mesg, destination, forceDebug );
}

void H245Thread :: processMesg ( H245 :: MultimediaSystemControlMessage & mesg, bool fromCaller,
	PTCPSocket * target, PTCPSocket * target2 ) {
// Task: to handle the given H.245 message and send it through the socket Target
	PSYSTEMLOG ( Info, "ProcessMesg" );
	bool changeTarget = false;
	if ( ! h -> handleMesg ( mesg, fromCaller, changeTarget ) )
		return;
	sendH245Mesg ( mesg, ( changeTarget ? target2 : target ), forceDebug );
}
