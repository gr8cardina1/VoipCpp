#pragma implementation
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "condvar.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include "ixcudpsocket.hpp"
#include "basegkclientthread.hpp"
#include "h323gkclientthread.hpp"
#include <stdexcept>
#include <limits>
#include <cstring>
#include "asn.hpp"
#include "h225.hpp"
#include "automutex.hpp"
#include "h323.hpp"
#include "AddrUtils.h"
#include "Log.h"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>
#include "ssconf.hpp"
#include "sqloutthread.hpp"

H323GkClientThread :: H323GkClientThread ( const ss :: string & n, const ss :: string & ln, int rs,
	const IntVector & l, const IntVector & b, const ss :: string & i, int p, const ss :: string & gkName ) :
	_ip ( i ), _name ( n ), _localName ( ln ), _terminalAlias ( ln ), _gkName ( gkName ),
	regDone ( mut ), sock ( WORD ( p ) ), _localIps ( l ), _bindIps ( b ), _port ( p ), _regPeriod ( rs ),
	regTries ( 0 ), registered ( false ), _discoveryComplete ( false ) {
	SetThreadName ( ( "H323GkClient " + gkName ).c_str ( ) );
	ss :: ostringstream os;
	os << "insert into GateKeepersState ( gkName, state, ip, port ) values " <<
		" ( \"" << _gkName << "\", \"UNREGISTERED\", \"" << _ip << "\", " << _port << " )";
	sqlOut -> add ( os.str ( ) );
	PSYSTEMLOG ( Info, "Constructing H323GkClientThread for " << gkName );
	Resume ( );
}

void H323GkClientThread :: Main ( ) {
	PSYSTEMLOG ( Info, "H323GkClientThread :: Main ( ) begin. " << _gkName );
	PIPSocket :: Address bindAddr = _bindIps [ 0 ];
	PIPSocket :: Address connectAddr ( _ip.c_str ( ) );
	while ( ! sock.PIPSocket :: Connect ( bindAddr, connectAddr ) ) {
		PSYSTEMLOG ( Error, "cant connect socket from " << bindAddr << " to " << connectAddr << ": " <<
			sock.GetErrorText ( ) );
		Sleep ( 3000 );
	}
	bool needUpdate = false;
	while ( sock.IsOpen ( ) ) {
		if ( ! registered && regTries < 1 ) {
			tryToRegister ( );
			needUpdate = true;
		}
		if ( ( PTime ( ) - lastRegistration ).GetSeconds ( ) >= _regPeriod ) {
			tryToRegister ( );
			needUpdate = true;
		}
		if ( needUpdate ) {
			ss :: ostringstream os;
			os << "update GateKeepersState set state = " << ( registered ? "\"REGISTERED\"" :
				"\"UNREGISTERED\"" ) << ", eventTime = from_unixtime( " <<
				PTime ( ).GetTimeInSeconds ( ) << " ), " << "expires = sec_to_time(" << _regPeriod
				<< " ) " << "where gkName = " << "\"" << _gkName << "\"";
			sqlOut -> add ( os.str ( ) );
			needUpdate = false;
		}
		Sleep ( 1000 );
	}
	PSYSTEMLOG ( Info, "H323GkClientThread :: Main ( ) end. " << _gkName );
}

void H323GkClientThread :: Close ( ) {
	PSYSTEMLOG ( Info, "H323GkClientThread :: Close" );
	ss :: ostringstream os;
	os << "delete from GateKeepersState where gkName = \"" << _gkName << "\"";
	sqlOut -> add ( os.str ( ) );
	sock.Close ( );
}

void H323GkClientThread :: updateState ( const ss :: string & n, const ss :: string & ln, int rs ) {
	AutoMutex am ( mut );
	_name = n;
	_localName = ln;
	_regPeriod = rs;
	regTries = 0;
}

ss :: string H323GkClientThread :: getLocalName ( ) {
	AutoMutex am ( mut );
	return _localName;
}

ss :: string H323GkClientThread :: getName ( ) {
	AutoMutex am ( mut );
	return _name;
}

void H323GkClientThread :: gotURQ ( ) {
	AutoMutex am ( mut );
	registered = false;
}

static ss :: string h225ProtocolID = "0.0.8.2250.0.4"; //!!!!! or .3 ?

int H323GkClientThread :: buildGRQ ( H225 :: RasMessage & mesg ) {
	mesg.setTag ( H225 :: RasMessage :: e_gatekeeperRequest );
	H225 :: GatekeeperRequest & grq = mesg;
	int n = getSeqNum ( );
	grq.m_requestSeqNum = n;
	grq.m_protocolIdentifier = h225ProtocolID;
	grq.m_rasAddress = AddrUtils :: convertToH225TransportAddr ( _localIps [ 0 ], 1719 );
	if ( ! _name.empty ( ) ) {
		grq.includeOptionalField ( H225 :: GatekeeperRequest :: e_gatekeeperIdentifier );
		grq.get_gatekeeperIdentifier ( ) = _name;
	}
	if ( ! _terminalAlias.empty ( ) ) {
		grq.includeOptionalField ( H225 :: GatekeeperRequest :: e_endpointAlias );
		grq.get_endpointAlias ( ).setSize ( 1 );
		H323 :: setAliasAddress ( _terminalAlias, grq.get_endpointAlias ( ) [ 0 ], H225 :: AliasAddress :: e_h323_ID );
	}
	return n;
}

bool H323GkClientThread :: sendGRQ ( int & n ) {
	H225 :: RasMessage mesg;
	n = buildGRQ ( mesg );
	H225 :: TransportAddress dest = AddrUtils :: convertToH225TransportAddr ( PIPSocket :: Address ( _ip.c_str ( ) ), WORD ( _port ) );
	Log -> logNote ( "Sending Gatekeeper Request" );
	Log -> logRasMsg ( mesg, OpengateLog :: Sending, dest );
	Asn :: ostream os;
	mesg.encode ( os );
	conf -> lockRas ( );
	return sock.Write ( os.str ( ).data ( ), PINDEX ( os.str ( ).size ( ) ) );
}

bool H323GkClientThread :: discover ( ) {
	int rasId;
	for ( int i = 0; i < 2; i ++ ) {
		if ( ! sendGRQ ( rasId ) ) { // eto ne exception safe
			conf -> unLockRas ( );
			PSYSTEMLOG ( Error, "SendGRQ: " << sock.GetErrorText ( ) );
			return false;
		}
		ss :: string gkid;
		int reason;
		_discoveryComplete = conf -> waitGrq ( rasId, gkid, reason );
		if ( ! gkid.empty ( ) ) {
			AutoMutex am ( mut );
			_name = gkid;
		}
		if ( _discoveryComplete )
			return true;
		if ( gkid.empty ( ) )
			break;
	}
	return _discoveryComplete;
}

int H323GkClientThread :: buildRRQ ( H225 :: RasMessage & mesg ) {
	mesg.setTag ( H225 :: RasMessage :: e_registrationRequest );
	H225 :: RegistrationRequest & rrq = mesg;
	int n = getSeqNum ( );
	rrq.m_requestSeqNum = n;
	rrq.m_protocolIdentifier = h225ProtocolID;
	rrq.m_discoveryComplete = _discoveryComplete;
	rrq.m_callSignalAddress.setSize ( _localIps.size ( ) );
	rrq.m_rasAddress.setSize ( _localIps.size ( ) );
	for ( unsigned i = 0; i < _localIps.size ( ); i ++ ) {
		rrq.m_callSignalAddress [ i ] = AddrUtils :: convertToH225TransportAddr ( _localIps [ i ], STD_SIGNALLING_PORT );
		rrq.m_rasAddress [ i ] = AddrUtils :: convertToH225TransportAddr ( _localIps [ i ], 1719 );
	}
	if ( ! _terminalAlias.empty ( ) ) {
		rrq.includeOptionalField ( H225 :: RegistrationRequest :: e_terminalAlias );
		rrq.get_terminalAlias ( ).setSize ( 1 );
		H323 :: setAliasAddress ( _terminalAlias, rrq.get_terminalAlias ( ) [ 0 ], H225 :: AliasAddress :: e_h323_ID );
	}
	rrq.includeOptionalField ( H225 :: RegistrationRequest :: e_timeToLive );
	rrq.get_timeToLive ( ) = _regPeriod;
	rrq.includeOptionalField ( H225 :: RegistrationRequest :: e_endpointIdentifier );
	rrq.get_endpointIdentifier ( ) = getLocalName ( );
	if ( ! _name.empty ( ) ) {
		rrq.includeOptionalField ( H225 :: RegistrationRequest :: e_gatekeeperIdentifier );
		rrq.get_gatekeeperIdentifier ( ) = _name;
	}
	return n;
}

bool H323GkClientThread :: sendRRQ ( int & n ) {
	H225 :: RasMessage mesg;
	n = buildRRQ ( mesg );
	H225 :: TransportAddress dest = AddrUtils :: convertToH225TransportAddr ( PIPSocket :: Address ( _ip.c_str ( ) ), WORD ( _port ) );
	Log -> logNote ( "Sending Registration Request" );
	Log -> logRasMsg ( mesg, OpengateLog :: Sending, dest );
	Asn :: ostream os;
	mesg.encode ( os );
	conf -> lockRas ( );
	return sock.Write ( os.str ( ).data ( ), PINDEX ( os.str ( ).size ( ) ) );
}

void H323GkClientThread :: tryToRegister ( ) {
	regTries ++;
	int rasId = 0;
	PTime now;
	ss :: string eid;
	for ( int i = 0; i < 2; i ++ ) {
		if ( ! sendRRQ ( rasId ) ) {
			conf -> unLockRas ( );
			PSYSTEMLOG ( Error, "SendRRQ: " << sock.GetErrorText ( ) );
			int t = std :: min ( regTries, 10 );
			AutoMutex am ( mut );
			registered = false;
			lastRegistration = now - PTimeInterval ( _regPeriod * 1000 - _regPeriod * t * 100 );
			PSYSTEMLOG ( Info, "regPeriod: " << _regPeriod << ", lastRegistration: " << lastRegistration );
			return;
		}
		now = PTime ( );
		int reason;
		conf -> waitRrq ( rasId, eid, reason );
		if ( ! eid.empty ( ) || reason != H225 :: RegistrationRejectReason :: e_discoveryRequired )
			break;
		if ( ! discover ( ) )
			break;
	}
	AutoMutex am ( mut );
	if ( eid.empty ( ) ) {
		PSYSTEMLOG ( Info, "Trying to receive RCF on own socket" );
		if ( ! receiveRCF ( rasId ) ) {
			registered = false;
			return;
		}
	}
	_localName = eid;
	lastRegistration = now;
	bool wasRegistered = registered;
	registered = true;
	PSYSTEMLOG ( Info, "Registered on H323 GK " << _gkName << " @ " << _ip << ':' << _port );
	regTries = 0;
	if ( ! wasRegistered )
		regDone.Signal ( );
}

void H323GkClientThread :: buildLRQ ( PUDPSocket & socket, const ss :: string & digits, H225 :: RasMessage & mesg, int & seqNum ) {
// Task: to build an LRQ message
	mesg.setTag ( H225 :: RasMessage :: e_locationRequest );
	H225 :: LocationRequest & lrq = mesg;
	PIPSocket :: Address replyAddr;
	WORD replyPort;
	socket.GetLocalAddress ( replyAddr, replyPort );
	if ( replyAddr == INADDR_ANY )
		replyAddr = _localIps [ 0 ];
	lrq.m_requestSeqNum = seqNum = getSeqNum ( );
	lrq.m_destinationInfo.setSize ( 1 );
	H323 :: setAliasAddress ( digits, lrq.m_destinationInfo [ 0 ] );
	lrq.m_replyAddress = AddrUtils :: convertToH225TransportAddr ( replyAddr, replyPort );
	lrq.includeOptionalField ( H225 :: LocationRequest :: e_sourceInfo );
	lrq.get_sourceInfo ( ).setSize ( 1 );
	ss :: string ln = getLocalName ( );
	H323 :: setAliasAddress ( ln, lrq.get_sourceInfo ( ) [ 0 ], H225 :: AliasAddress :: e_h323_ID );
//	lrq.includeOptionalField ( H225 :: LocationRequest :: e_endpointIdentifier );
//	lrq.get_endpointIdentifier ( ) = ln;
//	ss :: string nm = getName ( );
//	if ( ! nm.empty ( ) ) {
//		lrq.includeOptionalField ( H225 :: LocationRequest :: e_gatekeeperIdentifier );
//		lrq.get_gatekeeperIdentifier ( ) = nm;
//	}
	lrq.get_canMapAlias ( ) = true;
}

bool H323GkClientThread :: sendLRQ ( PUDPSocket & socket, const ss :: string & digits, int & seqNum ) {
// Task: to construct and send an LRQ to the given Gatekeeper to ask for the given alias
	H225 :: RasMessage lrq;
	buildLRQ ( socket, digits, lrq, seqNum );
	H225 :: TransportAddress dest = AddrUtils :: convertToH225TransportAddr ( PIPSocket :: Address ( _ip.c_str ( ) ), WORD ( _port ) );
	Log -> logNote ( "Sending Location Request" );
	Log -> logRasMsg ( lrq, OpengateLog :: Sending, dest );
	Asn :: ostream os;
	lrq.encode ( os );
	return socket.Write ( os.str ( ).data ( ), PINDEX ( os.str ( ).size ( ) ) );
}

bool H323GkClientThread :: decodeLCF ( const H225 :: RasMessage & mesg, H225 :: TransportAddress & destAddr, bool & retry ) {
// Task: to attempt to decode the given message as an LCF. If it is an LCF, return true
// fill in the parameters, and create an entry in our endpoint table.
	switch ( mesg.getTag ( ) ) {
		case H225 :: RasMessage :: e_locationConfirm:
			break;
		case H225 :: RasMessage :: e_locationReject:
			PSYSTEMLOG ( Info, "got lrj" );
			{
				const H225 :: LocationReject & lrj = mesg;
				if ( lrj.m_rejectReason.getTag ( ) == H225 :: LocationRejectReason :: e_notRegistered ) {
					AutoMutex am ( mut );
					registered = false;
					regDone.Wait ( 3000 );
					if ( registered )
						retry = true;
				}
			}
			return false;
		default:
			PSYSTEMLOG ( Error, "DecodeLCF: invalid tag: " << mesg.getTag ( ) );
			return false;
	}
	const H225 :: LocationConfirm & lcf = mesg;
	destAddr = lcf.m_callSignalAddress;
	return true;
}

bool H323GkClientThread :: receiveLCF ( int seqNum, PUDPSocket & socket, H225 :: TransportAddress & destAddr, bool & retry ) {
// Task: to receive a message on the given socket and if it is an LCF, return true and
// fill in the passed parameters
	static const int bufferSize = 4096;
	char buffer [ bufferSize ];
	// We need to set the read timeout to a sensible value.....
	PTimeInterval timeout ( 0, 3 ); // Try 3 seconds, should read from config
	while ( true ) {
		socket.SetReadTimeout ( timeout );
		PIPSocket :: Address addr;
		WORD port;
		if ( ! socket.ReadFrom ( buffer, bufferSize, addr, port ) ) {
			PSYSTEMLOG ( Error, "ReceiveLCF: can't read: " << socket.GetErrorText ( ) );
			return false;
		}
		try {
			Asn :: istream is ( Asn :: string ( buffer, socket.GetLastReadCount ( ) ) );
			H225 :: RasMessage reply ( is );
			if ( is.hasException ( ) )
				PSYSTEMLOG ( Error, "ReceiveLCF: can't decode message: " << is.getException ( ) );
			H225 :: TransportAddress sender = AddrUtils :: convertToH225TransportAddr ( addr, port );
			Log -> logNote ( "Received a reply to the Location Request" );
			Log -> logRasMsg ( reply, OpengateLog :: Receiving, sender );
			if ( reply.getTag ( ) == H225 :: RasMessage :: e_requestInProgress ) {
				H225 :: RequestInProgress & rip = reply;
				if ( int ( rip.m_requestSeqNum ) != seqNum ) {
					PSYSTEMLOG ( Error, "wrong seqNum: " << rip.m_requestSeqNum <<
						", should be " << seqNum );
					return false;
				}
				timeout = PTimeInterval ( rip.m_delay );
				continue;
			}
			return decodeLCF ( reply, destAddr, retry );
		} catch ( std :: exception & e ) {
			PSYSTEMLOG ( Error, "ReceiveLCF: can't decode message: " << e.what ( ) );
			return false;
		}
	}
}

bool H323GkClientThread :: receiveRCF ( int seqNum ) {
	static const int bufferSize = 4096;
	char buffer [ bufferSize ];
	// We need to set the read timeout to a sensible value.....
	PTimeInterval timeout ( 0, 3 ); // Try 3 seconds, should read from config
	sock.SetReadTimeout ( timeout );
	PIPSocket :: Address addr;
	WORD port;
	if ( ! sock.ReadFrom ( buffer, bufferSize, addr, port ) ) {
		PSYSTEMLOG ( Error, "ReceiveRCF: can't read: " << sock.GetErrorText ( ) );
		return false;
	}
	try {
		Asn :: istream is ( Asn :: string ( buffer, sock.GetLastReadCount ( ) ) );
		H225 :: RasMessage reply ( is );
		if ( is.hasException ( ) )
			PSYSTEMLOG ( Error, "ReceiveRCF: can't decode message: " << is.getException ( ) );
		H225 :: TransportAddress sender = AddrUtils :: convertToH225TransportAddr ( addr, port );
		Log -> logNote ( "Received a reply to the Registration Request" );
		Log -> logRasMsg ( reply, OpengateLog :: Receiving, sender );
		if ( reply.getTag ( ) != H225 :: RasMessage :: e_registrationConfirm ) {
			PSYSTEMLOG ( Error, "registration rejected" );
			return false;
		}
		H225 :: RegistrationConfirm & rcf = reply;
		if ( int ( rcf.m_requestSeqNum ) != seqNum ) {
			PSYSTEMLOG ( Error, "wrong seqNum: " << rcf.m_requestSeqNum << ", should be " << seqNum );
			return false;
		}
		return true;
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "ReceiveRCF: can't decode message: " << e.what ( ) );
		return false;
	}
}

bool H323GkClientThread :: getAddr ( const ss :: string & digits, PIPSocket :: Address & destAddr, int & destPort ) {
	PUDPSocket socket;
	if ( ! socket.Listen ( PIPSocket :: Address ( _bindIps [ 0 ] ) ) ) { // w/o Connect, because LCF may come from another GK
		PSYSTEMLOG ( Error, "getAddr. Error on Listen: " << socket.GetErrorText ( ) );
		return false;
	}
	socket.SetSendAddress ( PIPSocket :: Address ( _ip.c_str ( ) ), WORD ( _port ) );
	H225 :: TransportAddress dest;
	for ( int i = 0; i < 2; i ++ ) {
		int seqNum;
		if ( ! sendLRQ ( socket, digits, seqNum ) ) {
			PSYSTEMLOG ( Error, "SendLRQ: " << socket.GetErrorText ( ) );
			return false;
		}
		int selection = PSocket :: Select ( socket, socket, 3000 );
		if ( selection > 0 ) {
			PSYSTEMLOG ( Error, "gk select: " << PChannel :: GetErrorText ( PChannel :: Errors ( selection ) ) );
			return false;
		}
		if ( selection == 0 ) {
			PSYSTEMLOG ( Error, "gk select: timeout" );
			return false;
		}
		bool retry = false;
		if ( receiveLCF ( seqNum, socket, dest, retry ) )
			break;
		if ( ! retry )
			return false;
	}
	if ( dest.getTag ( ) != H225 :: TransportAddress :: e_ipAddress ) {
		PSYSTEMLOG ( Error, "gk returned address which is not an IP address" );
		return false;
	}
	H225 :: TransportAddress_ipAddress & destIP = dest;
	WORD port;
	AddrUtils :: convertToIPAddress ( destIP, destAddr, port );
	destPort = port;
	return true;
}

int H323GkClientThread :: getSeqNum ( ) {
	static unsigned short int seqNum = 0;
	return __sync_add_and_fetch ( & seqNum, 1 );
}
