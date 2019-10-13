#pragma implementation
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "condvar.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include "ixcudpsocket.hpp"
#include "basegkclientthread.hpp"
#include "SIPAuthenticateValidator.hpp"
#include "sipgkclientthread.hpp"
#include "automutex.hpp"
#include "Log.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include "sip.hpp"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <tr1/cstdint>
#include "md5.hpp"
#include <ptlib/svcproc.h>
#include "sqloutthread.hpp"
#include <cstring>

SipGkClientThread :: SipGkClientThread ( const ss :: string & name, const ss :: string & localName, int regPeriod,
	const IntVector & localIps, const IntVector & bindIps, const ss :: string & ip, const ss :: string & pswd,
	int port, const ss :: string & gkName ) : _ip ( ip ), _name ( name ), _localName ( localName ),
	_terminalAlias ( localName ), _gkName ( gkName ), regDone ( mut ), sock ( WORD ( port ) ), _localIps ( localIps ),
	_bindIps ( bindIps ), _port ( port ), _regPeriod ( regPeriod ), regTries ( 0 ), registered ( false ),
	_pswd ( pswd ) {
	SetThreadName ( ( "SipGkClient " + gkName ).c_str ( ) );
	ss :: ostringstream os;
	os << "insert into GateKeepersState ( gkName, state, ip, port ) values " <<
		" ( \"" << _gkName << "\", \"UNREGISTERED\", \"" << _ip << "\", " << port << " )";
	sqlOut -> add ( os.str ( ) );
	PSYSTEMLOG ( Info, "Constructing SipGkClientThread for " << gkName );
	PIPSocket :: Address lIp = _localIps [ 0 ];
	PString t = lIp.AsString ( );
	_localIp = static_cast < const char * > ( t );
	_callId = _generateUID ( ) + '@' + _localIp;
	Resume ( );
}

void SipGkClientThread :: Main ( ) {
	PSYSTEMLOG ( Info, "SipGkClientThread :: Main ( ) begin. " << _gkName );
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
	PSYSTEMLOG ( Info, "SipGkClientThread :: Main ( ) end. " << _gkName );
}

void SipGkClientThread :: Close ( ) {
	PSYSTEMLOG ( Info, "SipGkClientThread :: Close" );
	ss :: ostringstream os;
	os << "delete from GateKeepersState where gkName = \"" << _gkName << "\"";
	sqlOut -> add ( os.str ( ) );
	sock.Close ( );
}

void SipGkClientThread :: updateState ( const ss :: string & n, const ss :: string & ln, int rs ) {
	AutoMutex am ( mut );
	_name = n;
	_localName = ln;
	_regPeriod = rs;
	regTries = 0;
}

void SipGkClientThread :: _buildRRQ ( int cSeq, const ss :: string & authLine, SIP :: PDU & mesg ) {
	SIP :: URL from, to, contact;
	ss :: string tag, branch;
	ss :: string via;
	to.setHostName ( _ip );
	to.setPort ( short ( _port ) );
	to.setUserName ( _terminalAlias );
	from.setUserName ( _terminalAlias );
	from.setHostName ( _ip );
	from.setPort ( 5060 );
	tag = _generateUID ( );
	from.setSipParam ( "tag", tag );
	branch = "z9hG4bK" + _generateUID ( );
	contact.setUserName ( _terminalAlias );
	contact.setHostName ( _localIp );
	contact.setPort ( 5060 );
	via = "SIP/2.0/UDP " + _localIp + ";branch=" + branch;

	mesg.setMethod ( SIP :: PDU :: mRegister );
	mesg.setURI ( "sip:" + _ip );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( via );
	mime.setFrom ( from.str ( ) );
	mime.setTo ( to.str ( ) );
	mime.setCallID ( _callId );
	mime.setContact ( contact.bracketShortForm ( ) );
	ss :: ostringstream os1, os2;
	os1 << cSeq << " REGISTER";
	mime.setCSeq ( os1.str ( ) );
	os2 << _regPeriod;
	mime.setExpires ( os2.str ( ) );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	if ( authLine != "" ) {	// adding authorization
		ss :: string uri = "sip:" + _localIp;
		ss :: string resp = _validator.buildResponse ( authLine, _terminalAlias, _pswd, uri,
			"REGISTER", true/*conf -> isUseCNonce( _ip )*/ );
		mime.setAuthorization ( resp );
	}
}

bool SipGkClientThread :: _sendRRQ ( ) {
	SIP :: PDU mesg;
	_buildRRQ ( getSeqNum ( ), _authLine, mesg );
	Log -> logNote ( "Sending Registration Request" );
	Log -> LogSIPMsg ( mesg, OpengateLog :: Sending, PIPSocket :: Address ( _ip.c_str ( ) ), WORD ( _port ) );
	return mesg.write ( sock );
}

bool SipGkClientThread :: _receiveMesg ( SIP :: PDU & mesg ) {
	PTimeInterval timeout ( 0, 3 );
	sock.SetReadTimeout ( timeout );
	if ( ! mesg.read ( sock ) ) {
		PSYSTEMLOG ( Error, "SipGkClientThread :: Receive: can't read: " << sock.GetErrorText ( ) );
		return false;
	}
	PIPSocket::Address peerAddr;
	WORD peerPort;
	sock.GetLastReceiveAddress ( peerAddr, peerPort );
	Log -> logNote ( "Received a reply to the Registration Request" );
	Log -> LogSIPMsg ( mesg, OpengateLog :: Receiving, peerAddr, peerPort );
	return true;
}

bool SipGkClientThread :: _handleOK ( SIP :: PDU & /*mesg*/ ) {
	registered = true;
	PSYSTEMLOG ( Info, "Registered on SipRegistrar " << _gkName << " @ " << _ip << ':' << _port );
	return true;
}

bool SipGkClientThread :: _handleError ( SIP :: PDU & mesg ) {
	if ( mesg.getStatusCode ( ) == SIP :: PDU :: scFailureUnAuthorised ) {
		SIP :: MIMEInfo & mime = mesg.getMIME ( );
		_authLine = mime.getWWWAuthenticate ( );
		return true;
	}
	return false;
}

bool SipGkClientThread :: _handleMesg ( SIP :: PDU & mesg ) {
	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scSuccessfulOK )
		return _handleOK ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) > SIP :: PDU :: scSuccessfulOK )
		return _handleError ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scInformationTrying )
		return false;
	PSYSTEMLOG ( Error, "Unexpected message!!!" );
	return false;
}

void SipGkClientThread :: tryToRegister ( ) {
	regTries ++;
	registered = false;
	_authLine = "";
	int retry = 0;
	lastRegistration = PTime ( );
	while ( retry < 3 && ! registered ) {
		retry ++;
		if ( ! _sendRRQ ( ) ) {
			PSYSTEMLOG ( Error, "SipGkClientThread :: SendRRQ: " << sock.GetErrorText ( ) );
			return;
		}
		SIP :: PDU reply;
		do {
			if ( ! _receiveMesg ( reply ) ) {
				PSYSTEMLOG ( Error, "SipGkClientThread :: RecvAns: " << sock.GetErrorText ( ) );
				return;
			}
		} while ( ! _handleMesg ( reply ) );
	}
	return;
}

ss :: string SipGkClientThread :: _generateUID ( ) {
	ss :: ostringstream os;
	os << ( PTime ( ).GetTimestamp ( ) + ( rand ( ) % 1000 ) ) << '@' << _ip << ':' << _port;
	HASH binCoded;
	HASHHEX charCoded;
	std :: memcpy ( binCoded, md5Sum ( os.str ( ) ).data ( ), 16 );
	CvtHex ( binCoded, charCoded );
	return charCoded;
}

bool SipGkClientThread :: getAddr ( const ss :: string & /*digits*/, PIPSocket :: Address & /*destAddr*/, int & /*destPort*/ ) {
	PSYSTEMLOG ( Info, "getAddr NOT IMPLEMENETED!!!" );
	return false;
}

int SipGkClientThread :: getSeqNum ( ) {
	static PMutex mut;
	static int seqNum = 0;
	AutoMutex am ( mut );
	return ++ seqNum;
}

void SipGkClientThread :: CvtHex( const HASH Bin, HASHHEX Hex ) {
	for ( unsigned short i = 0; i < sizeof ( HASH ); i++ ) {
		unsigned char j = char ( ( Bin[i] >> 4 ) & 0xf );
		if (j <= 9) Hex[i*2] = char (j + '0');
		else Hex[i*2] = char (j + 'a' - 10);
		j = char ( Bin[i] & 0xf );
		if (j <= 9) Hex[i*2+1] = char (j + '0');
		else Hex[i*2+1] = char (j + 'a' - 10);
	}
	Hex [ sizeof ( HASHHEX ) - 1 ] = '\0';
}


