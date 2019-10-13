#pragma implementation "mgcp.hpp"
#include "ss.hpp"
#include "mgcp.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "allocatable.hpp"
#include "ixcudpsocket.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include "sdp.hpp"
#include <ptlib/svcproc.h>
#include "pointer.hpp" //for safeDel
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/io/ios_state.hpp>

namespace MGCP {

void MIMEInfo :: swap ( MIMEInfo & m ) {
	headers.swap ( m.headers );
}

void MIMEInfo :: addHeader ( const ss :: string & n, const ss :: string & v ) {
	headers [ n ].push_back ( v );
}

void MIMEInfo :: removeHeader ( const ss :: string & n ) {
	headers.erase ( n );
}

void MIMEInfo :: printOn ( std :: ostream & os ) const {
	for ( StringStringVectorMap :: const_iterator i = headers.begin ( );
		i != headers.end ( ); ++ i ) {
		for ( StringVector :: const_iterator j = i -> second.begin ( );
			j != i -> second.end ( ); ++ j ) {
			os << i -> first << ": ";
			if ( i -> first == "A" )
				os << Capabilities ( * j );
			else
				os << * j;
			os << "\r\n";
		}
	}
}

void MIMEInfo :: readFrom ( std :: istream & is ) {
	headers.clear ( );
	ss :: string s;
	while ( getline ( is, s ) ) {
		if ( s.size ( ) && s [ s.size ( ) - 1 ] == '\r' )
			s.erase ( s.size ( ) - 1 );
		ss :: string :: size_type pos = s.find ( ':' );
		if ( pos == ss :: string :: npos )
			return;
		ss :: string key = s.substr ( 0, pos );
		ss :: string val = boost :: algorithm :: trim_copy ( s.substr ( pos + 1 ) );
		addHeader ( key, val );
	}
}

void MIMEInfo :: clear ( ) {
	headers.clear ( );
}

bool MIMEInfo :: getHeader ( const ss :: string & n, StringVector & s ) const {
	StringStringVectorMap :: const_iterator i = headers.find ( n );
	if ( i == headers.end ( ) )
		return false;
	s = i -> second;
	return true;
}

bool MIMEInfo :: getHeader ( const ss :: string & n, ss :: string & s ) const {
	StringStringVectorMap :: const_iterator i = headers.find ( n );
	if ( i == headers.end ( ) )
		return false;
	s = i -> second [ 0 ];
	return true;
}

bool MIMEInfo :: hasHeader ( const ss :: string & n ) const {
	return headers.find ( n ) != headers.end ( );
}

bool MIMEInfo :: getSpecificEndPointId ( StringVector & s ) const {
	return getHeader ( "Z", s );
}

bool MIMEInfo :: getSpecificEndPointId ( ss :: string & s ) const {
	return getHeader ( "Z", s );
}

bool MIMEInfo :: getConnectionId ( ss :: string & s ) const {
	return getHeader ( "I", s );
}

bool MIMEInfo :: hasConnectionId ( ) const {
	return hasHeader ( "I" );
}

bool MIMEInfo :: getSecondConnectionId ( ss :: string & s ) const {
	return getHeader ( "I2", s );
}

bool MIMEInfo :: getRestartMethod ( ss :: string & s ) const {
	return getHeader ( "RM", s );
}

bool MIMEInfo :: getRestartDelay ( int & d ) const {
	ss :: string s;
	if ( ! getHeader ( "RD", s ) )
		return false;
	d = std :: atoi ( s.c_str ( ) );
	return true;
}

bool MIMEInfo :: getRequestIdentifier ( ss :: string & s ) const {
	return getHeader ( "X", s );
}

bool MIMEInfo :: getRequestedEvents ( ss :: string & s ) const {
	return getHeader ( "R", s );
}

bool MIMEInfo :: getCapabilities ( Capabilities & c ) const {
	ss :: string s;
	if ( ! getHeader ( "A", s ) )
		return false;
	c = s;
	return true;
}

bool MIMEInfo :: getCapabilities ( std :: vector < Capabilities, __SS_ALLOCATOR < Capabilities > > & c ) const {
	StringVector v;
	if ( ! getHeader ( "A", v ) )
		return false;
	c.clear ( );
	for ( unsigned i = 0; i < v.size ( ); i ++ )
		c.push_back ( Capabilities ( v [ i ] ) );
	return true;
}

bool MIMEInfo :: getObservedEvents ( ss :: string & s ) const {
	return getHeader ( "O", s );
}

bool MIMEInfo :: getResponseAck ( ss :: string & s ) const {
	return getHeader ( "K", s );
}

bool MIMEInfo :: hasResponseAck ( ) const {
	return hasHeader ( "K" );
}

bool MIMEInfo :: getEventStates ( ss :: string & s ) const {
	return getHeader ( "ES", s );
}

void MIMEInfo :: setMode ( const ss :: string & v ) {
	removeHeader ( "M" );
	addHeader ( "M", v );
}

void MIMEInfo :: setCallId ( const ss :: string & v ) {
	addHeader ( "C", v );
}

void MIMEInfo :: setSecondEndPointId ( const ss :: string & v ) {
	addHeader ( "Z2", v );
}

void MIMEInfo :: setRequestIdentifier ( const ss :: string & v ) {
	addHeader ( "X", v );
}

void MIMEInfo :: setRequestedEvents ( const ss :: string & v ) {
	addHeader ( "R", v );
}

void MIMEInfo :: setCapabilities ( const Capabilities & v ) {
	addHeader ( "A", v.str ( ) );
}

void MIMEInfo :: setRequestedInfo ( const ss :: string & v ) {
	addHeader ( "F", v );
}

void MIMEInfo :: setPersistentEvents ( const ss :: string & v ) {
	addHeader ( "B/PR", v );
}

void MIMEInfo :: setConnectionId ( const ss :: string & v ) {
	addHeader ( "I", v );
}

void MIMEInfo :: setResponseAck ( const ss :: string & v ) {
	addHeader ( "K", v );
}

void MIMEInfo :: setSignalRequests ( const ss :: string & v ) {
	addHeader ( "S", v );
}

void MIMEInfo :: setDigitMap ( const ss :: string & v ) {
	addHeader ( "D", v );
}

void MIMEInfo :: setNotifiedEntity ( const ss :: string & v ) {
	addHeader ( "N", v );
}

void MIMEInfo :: setLocalConnectionOptions ( const ss :: string & v ) {
	addHeader ( "L", v );
}

void MIMEInfo :: removeRequestedEvents ( ) {
	removeHeader ( "R" );
}

void MIMEInfo :: removeRequestedInfo ( ) {
	removeHeader ( "F" );
}

Capabilities :: Capabilities ( const ss :: string & s ) {
	ss :: istringstream is ( s );
	readFrom ( is );
}

Capabilities & Capabilities :: operator= ( const ss :: string & s ) {
	clear ( );
	ss :: istringstream is ( s );
	readFrom ( is );
	return * this;
}

ss :: string Capabilities :: str ( ) const {
	ss :: ostringstream os;
	printOn ( os );
	return os.str ( );
}

void Capabilities :: addHeader ( const ss :: string & n, const ss :: string & v ) {
	headers [ n ].push_back ( v );
}

void Capabilities :: printOn ( std :: ostream & os ) const {
	for ( StringStringVectorMap :: const_iterator i = headers.begin ( );
		i != headers.end ( ); ++ i ) {
		if ( i != headers.begin ( ) )
			os << ", ";
		os << i -> first;
		for ( StringVector :: const_iterator j = i -> second.begin ( );
			j != i -> second.end ( ); ++ j ) {
			if ( j == i -> second.begin ( ) )
				os << ':';
			else
				os << ';';
			os << * j;
		}
	}
}

void Capabilities :: readFrom ( std :: istream & is ) {
	headers.clear ( );
	ss :: string s;
	while ( getline ( is, s, ',' ) ) {
		boost :: algorithm :: trim ( s );
		if ( s.empty ( ) )
			continue;
		ss :: string :: size_type pos = s.find ( ':' );
		if ( pos == ss :: string :: npos )
			return;
		ss :: string key = s.substr ( 0, pos );
		ss :: string val = boost :: algorithm :: trim_copy ( s.substr ( pos + 1 ) );
		ss :: istringstream tis ( val );
		ss :: string t;
		while ( getline ( tis, t, ';' ) ) {
			boost :: algorithm :: trim ( t );
			if ( t.empty ( ) )
				continue;
			addHeader ( key, t );
		}
	}
}

void Capabilities :: clear ( ) {
	headers.clear ( );
}

bool Capabilities :: getHeader ( const ss :: string & n, StringVector & s ) const {
	StringStringVectorMap :: const_iterator i = headers.find ( n );
	if ( i == headers.end ( ) )
		return false;
	s = i -> second;
	return true;
}

bool Capabilities :: getHeader ( const ss :: string & n, ss :: string & s ) const {
	StringStringVectorMap :: const_iterator i = headers.find ( n );
	if ( i == headers.end ( ) )
		return false;
	s = i -> second [ 0 ];
	return true;
}

unsigned PDU :: curTrans = 0;

unsigned PDU :: getNewId ( ) {
	if ( ++ curTrans > 999999999 )
		curTrans = 1;
	return curTrans;
}

const ss :: string & PDU :: getVerbName ( ) const {
	static ss :: string names [ ] = {
		"EPCF",
		"CRCX",
		"MDCX",
		"DLCX",
		"RQNT",
		"NTFY",
		"AUEP",
		"AUCX",
		"RSIP",
		"MESG"
	};
	return names [ verb ];
}

const char * PDU :: getComment ( ReturnCodes rc ) {
	switch ( rc ) {
		case rcResponseAck:
			return "OK";
		case rcProvisional:
			return "Pending";
		case rcOK:
			return "OK";
		case rcDeleted:
			return "Connection was deleted";
		case rcOffHook:
			return "The phone is already off hook";
		case rcOnHook:
			return "The phone is already on hook";
		case rcUnknownEndpoint:
			return "Unknown endpoint";
		case rcNoResources:
			return "Endpoint does not have sufficient resources";
		case rcAllOfTooComplicated:
			return "\"All of\" wildcard too complicated";
		case rcUnknownCommand:
			return "Unknown or unsupported command";
		case rcProtocolError:
			return "Some unspecified protocol error was detected";
		case rcCantDetect:
			return "Gateway is not equipped to detect one of the requested events";
		case rcNoSuchEvent:
			return "No such event or signal";
		case rcNoRemoteConnection:
			return "Missing RemoteConnectionDescriptor";
		case rcIncompatibleProtocolVersion:
			return "Incompatible protocol version";
		case rcUnknownRestartMethod:
			return "Unknown or unsupported RestartMethod";
		case rcInvalid:
			return "";
	}
	return "";
}

const ss :: string & PDU :: getComment ( ) const {
	return comment;
}

PDU :: PDU ( const ss :: string & e, const ss :: string & g, const ss :: string & i, unsigned short p, Verbs v ) :
	id ( getNewId ( ) ), ep ( e ), gw ( g ), ip ( i ), port ( p ), verb ( v ), rc ( rcInvalid ), sdp ( 0 ) { }

PDU :: PDU ( ReturnCodes r, unsigned i, const ss :: string & a, unsigned short p ) : id ( i ), ip ( a ), port ( p ),
	verb ( vNum ), rc ( r ), comment ( getComment ( r ) ), sdp ( 0 ) { }
	// dobavit gw/ep chtobi response cache rabotal bez problem ?

PDU :: PDU ( const PDU & p ) : id ( p.id ), ep ( p.ep ), gw ( p.gw ), ip ( p.ip ), port ( p.port ), verb ( p.verb ),
	rc ( p.rc ), comment ( p.comment ), mime ( p.mime ),
	sdp ( p.sdp ? new SDP :: SessionDescription ( * p.sdp ) : 0 ) { }

PDU :: PDU ( const ss :: string & i, unsigned short p ) : id ( 0 ), ip ( i ), port ( p ), verb ( vNum ),
	rc ( rcInvalid ), sdp ( 0 ) { }

PDU & PDU :: operator= ( const PDU & p ) {
	PDU ( p ).swap ( * this );
	return * this;
}

void PDU :: swap ( PDU & p ) {
	std :: swap ( id, p.id );
	ep.swap ( p.ep );
	gw.swap ( p.gw );
	ip.swap ( p.ip );
	std :: swap ( port, p.port );
	std :: swap ( verb, p.verb );
	std :: swap ( rc, p.rc );
	comment.swap ( p.comment );
	mime.swap ( p.mime );
	std :: swap ( sdp, p.sdp );
}

ss :: string PDU :: str ( ) const {
	ss :: ostringstream os;
	printOn ( os );
	return os.str ( );
}

void PDU :: printOn ( std :: ostream & os ) const {
	boost :: io :: ios_locale_saver ls ( os, std :: locale :: classic ( ) );
	if ( rc != rcInvalid ) {
		os << rc << ' ' << id;
		if ( ! comment.empty ( ) )
			os << ' ' << comment;
	} else
		os << getVerbName ( ) << ' ' << id << ' ' << ep << '@' << gw << " MGCP 1.0";
	os << "\r\n" << mime;
	if ( sdp )
		os << "\r\n" << * sdp;
}

void PDU :: readFrom ( std :: istream & is ) {
	ss :: string s;
	if ( ! getline ( is, s ) ) {
		PSYSTEMLOG ( Error, "cannot read command line" );
		return;
	}
	ss :: istringstream si ( s );
	ss :: string v, e, m, vers;
	si >> v;
	if ( std :: isdigit ( v [ 0 ] ) )
		rc = ReturnCodes ( std :: atoi ( v.c_str ( ) ) );
	else
		rc = rcInvalid;
	if ( rc == rcInvalid ) {
		si >> id >> e >> m >> vers;
		ss :: string :: size_type pos = e.find ( '@' );
		if ( m != "MGCP" || vers != "1.0" || pos == ss :: string :: npos ) {
			PSYSTEMLOG ( Error, "mailformed command line : " << s );
			return;
		}
		ep = e.substr ( 0, pos );
		gw = e.substr ( pos + 1 );
		boost :: algorithm :: to_lower ( gw );
		boost :: algorithm :: to_upper ( v );
		if ( v == "EPCF" )
			verb = vEndpointConfiguration;
		else if ( v == "CRCX" )
			verb = vCreateConnection;
		else if ( v == "MDCX" )
			verb = vModifyConnection;
		else if ( v == "DLCX" )
			verb = vDeleteConnection;
		else if ( v == "RQNT" )
			verb = vNotificationRequest;
		else if ( v == "NTFY" )
			verb = vNotify;
		else if ( v == "AUEP" )
			verb = vAuditEndpoint;
		else if ( v == "AUCX" )
			verb = vAuditConnection;
		else if ( v == "RSIP" )
			verb = vRestartInProgress;
		else if ( v == "MESG" )
			verb = vMessage;
		else
			verb = vNum;
	} else {
		verb = vNum;
		ep.clear ( );
		gw.clear ( );
		comment.clear ( );
		si >> id;
		getline ( si, comment );
		boost :: algorithm :: trim ( comment );
	}
	mime.clear ( );
	safeDel ( sdp );
	if ( is >> mime ) {
		ss :: ostringstream os;
		ss :: string s;
		while ( getline ( is, s ) )
			os << s << '\n';
		os.str ( ).swap ( s );
		sdp = new SDP :: SessionDescription ( s.begin ( ), s.end ( ) );
	}
}

PDU :: ~PDU ( ) {
	delete sdp;
}

const MIMEInfo & PDU :: getMIME ( ) const {
	return mime;
}

MIMEInfo & PDU :: getMIME ( ) {
	return mime;
}

void PDU :: setMIME ( const MIMEInfo & m ) {
	mime = m;
}

void PDU :: setEndpoint ( const ss :: string & s ) {
	ep = s;
}

void PDU :: newId ( ) {
	id = getNewId ( );
}

void PDU :: setVerb ( Verbs v ) {
	verb = v;
	rc = rcInvalid;
}

void PDU :: setCode ( ReturnCodes c, const ss :: string & com ) {
	rc = c;
	if ( com.empty ( ) )
		comment = getComment ( c );
	else
		comment = com;
	verb = vNum;
}

void PDU :: setSDP ( SDP :: SessionDescription * s ) {
	delete sdp;
	sdp = s;
}

void PDU :: setSDP ( const SDP :: SessionDescription & s ) {
	SDP :: SessionDescription * t = new SDP :: SessionDescription ( s );
	delete sdp;
	sdp = t;
}

unsigned PDU :: getId ( ) const {
	return id;
}

PDU :: ReturnCodes PDU :: getCode ( ) const {
	return rc;
}

PDU :: Verbs PDU :: getVerb ( ) const {
	return verb;
}

ss :: string PDU :: getEndpointGw ( ) const {
	return ep + '@' + gw;
}

const ss :: string & PDU :: getIp ( ) const {
	return ip;
}

const SDP :: SessionDescription * PDU :: getSDP ( ) const {
	return sdp;
}

}

void makeResponse ( MGCP :: PduVector & pdus, const MGCP :: PDU & pdu, MGCP :: PDU :: ReturnCodes rc ) {
	MGCP :: PDU ret ( pdu );
	ret.setSDP ( 0 );
	ret.setCode ( rc );
	ret.getMIME ( ).clear ( );
	pdus.push_back ( ret );
}
