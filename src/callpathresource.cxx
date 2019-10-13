#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptclib/http.h>
#include "callpathresource.hpp"
#include <boost/tokenizer.hpp>
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "sourcedata.hpp"
#include "signallingoptions.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include <tr1/memory>
#include <tr1/functional>
#include "smartguard.hpp"
#include "pointer.hpp"
#include "moneyspool.hpp"
#include "commoncalldetails.hpp"
#include <ptclib/pxml.h>

CallPathResource :: CallPathResource ( ) :
	PHTTPResource ( "callpath.xml", "text/xml" ) { }

static void doRegister ( const ss :: string & login, const ss :: string & ip, int port, bool h323, bool isCard ) {
	static PUDPSocket sock;
	PIPSocket :: Address addr ( ip.c_str ( ) );
	StringSet neededNumbers, onlineNumbers;
	IcqContactsVector icqContacts;
	conf -> registerPeer ( login, addr, port, h323, false, PTime ( ) + 1000, isCard, addr, port, neededNumbers,
		onlineNumbers, 0, sock, icqContacts );
}

PString CallPathResource :: LoadText ( PHTTPRequest & request ) {
	const PStringToString & q = request.url.GetQueryVars ( );
	PString * s = q.GetAt ( "ip" );
	if ( ! s ) {
		PSYSTEMLOG ( Error, "no ip in callpath" );
		return "";
	}
	ss :: string ip = static_cast < const char * > ( * s );
	s = q.GetAt ( "port" );
	if ( ! s ) {
		PSYSTEMLOG ( Error, "no port in callpath" );
		return "";
	}
	int port = std :: atoi ( static_cast < const char * > ( * s ) );
	s = q.GetAt ( "dialedDigits" );
	if ( ! s ) {
		PSYSTEMLOG ( Error, "no dialedDigits in callpath" );
		return "";
	}
	ss :: string dialedDigits = static_cast < const char * > ( * s );
	s = q.GetAt ( "price" );
	if ( ! s ) {
		PSYSTEMLOG ( Error, "no price in callpath" );
		return "";
	}
	int price = std :: atoi ( static_cast < const char * > ( * s ) );
	s = q.GetAt ( "proto" );
	if ( ! s ) {
		PSYSTEMLOG ( Error, "no proto in callpath" );
		return "";
	}
	bool h323 = * s == "h323";
	s = q.GetAt ( "incomingCodecs" );
	if ( ! s ) {
		PSYSTEMLOG ( Error, "no incomingCodecs in callpath" );
		return "";
	}
	boost :: char_separator < char > sep ( "," );
	typedef boost :: tokenizer < boost :: char_separator < char >, const char *, ss :: string > Tok;
	const char * b = static_cast < const char * > ( * s );
	Tok tok ( b, b + s -> GetSize ( ) - 1, sep );
	CodecInfoContainer incomingCodecs;
	std :: copy ( tok.begin ( ), tok.end ( ), std :: back_inserter ( incomingCodecs ) );
	bool isCard = false;
	s = q.GetAt ( "login" );
	ss :: string login;
	if ( s ) {
		login = static_cast < const char * > ( * s );
		s = q.GetAt ( "isCard" );
		if ( ! s ) {
			PSYSTEMLOG ( Error, "no isCard in callpath" );
			return "";
		}
		isCard = * s == "true";
		doRegister ( login, ip, port, h323, isCard );
	} else if ( ! conf -> isValidInPeerAddress ( ip ) ) {
		PSYSTEMLOG ( Error, "invalid inpeer address in callpath" );
		return "";
	}
	if ( ! conf -> tryTake ( ) ) {
		PSYSTEMLOG ( Error, "tryTake failed" );
		return "";
	}
	CommonCallDetails common;
	common.setFromNat ( false );
	common.setCallerIp ( ip );
	common.setCallerPort ( port );
	common.setConfId ( "0123456789" );
	if ( isCard ) {
		common.source ( ).acctn = login;
		common.source ( ).type = SourceData :: card;
	} else
		common.source ( ).type = SourceData :: inbound;
	common.setDialedDigits ( dialedDigits );
	common.source ( ).price = price;
	common.incomingCodecs ( ).c.swap ( incomingCodecs );
	OutChoiceDetailsVectorVector choiceForks;
	StringVector forkOutAcctns;
	conf -> getCallInfo ( choiceForks, forkOutAcctns, common, true, true, true );
	common.release ( );
	PXML xml ( PXML :: Indent | PXML :: NewLineAfterElement );
	PXMLElement * elem = new PXMLElement ( 0, "callpath" );
	xml.SetRootElement ( elem );
	ss :: ostringstream os;
	os << common.getSource ( ).price / 100000.0;
	elem -> SetAttribute ( "price", os.str ( ).c_str ( ) );
	os.str ( ss :: string ( ) );
	elem -> SetAttribute ( "code", common.getSource ( ).code.c_str ( ) );
	bool legs = false;
	if ( h323 ) {
		if ( ! common.getIncomingCodecs ( ).c.empty ( )	&& ( choiceForks.size ( ) > 1 ||
			common.getUseNormalizer ( ) || hasSipChoices ( common.getChoices ( ) ) ) )
			legs = true;
	} else {
//		if ( ! common.getIncomingCodecs ( ).c.empty ( ) && ( choiceForks.size ( ) > 1 ||
//			common.getUseNormalizer ( ) || hasH323Choices ( common.getChoices( ) ) ) )
			legs = true;
	}
	elem -> SetAttribute ( "proxy", legs ? "false" : "true" );
	elem -> SetAttribute ( "primeCost", common.getSource ( ).allPrice ? "true" : "false" );
	for ( std :: size_t i = 0; i < choiceForks.size ( ); i ++ ) {
		PXMLElement * elem2 = new PXMLElement ( elem, "fork" );
		elem -> AddChild ( elem2 );
		elem2 -> SetAttribute ( "outAcctn", forkOutAcctns [ i ].c_str ( ) );
		const OutChoiceDetailsVector & v = choiceForks [ i ];
		for ( std :: size_t j = 0; j < v.size ( ); j ++ ) {
			PXMLElement * elem3 = new PXMLElement ( elem2, "choice" );
			elem2 -> AddChild ( elem3 );
			const OutChoiceDetails & c = v [ j ];
			elem3 -> SetAttribute ( "realDigits", c.getRealDigits ( ).c_str ( ) );
			elem3 -> SetAttribute ( "sentDigits", c.getSentDigits ( ).c_str ( ) );
			elem3 -> SetAttribute ( "code", c.getCode ( ).c_str ( ) );
			os << c.getPeer ( );
			elem3 -> SetAttribute ( "peer", os.str ( ).c_str ( ) );
			os.str ( ss :: string ( ) );
			elem3 -> SetAttribute ( "ip", c.getIp ( ).c_str ( ) );
			os << c.getPort ( );
			elem3 -> SetAttribute ( "port", os.str ( ).c_str ( ) );
			os.str ( ss :: string ( ) );
			os << c.getLim ( );
			elem3 -> SetAttribute ( "limit", os.str ( ).c_str ( ) );
			os.str ( ss :: string ( ) );
			elem3 -> SetAttribute ( "directRTP", c.getDirectRTP ( ) ? "true" : "false" );
			elem3 -> SetAttribute ( "proto", c.getType ( ) == ptH323 ? "h323" :
				( c.getType ( ) == ptSip ? "sip" : "mgcp" ) );
			os << c.getPrice ( ) / 100000.0;
			elem3 -> SetAttribute ( "price", os.str ( ).c_str ( ) );
			os.str ( ss :: string ( ) );
			os << c.getConnectPrice ( ) / 100000.0;
			elem3 -> SetAttribute ( "connectPrice", os.str ( ).c_str ( ) );
			os.str ( ss :: string ( ) );
			if ( forkOutAcctns [ i ].empty ( ) )
				continue;
			PXMLElement * elem4 = new PXMLElement ( elem3, "outCardDetails" );
			elem3 -> AddChild ( elem4 );
			const OutCardDetails & ocd = c.getOutCardDetails ( );
			elem4 -> SetAttribute ( "digits", ocd.getDigits ( ).c_str ( ) );
			elem4 -> SetAttribute ( "code", ocd.getCode ( ).c_str ( ) );
			elem4 -> SetAttribute ( "redirect", ocd.getRedirect ( ) ? "true" : "false" );
			os << ocd.getPrice ( ) / 100000.0;
			elem4 -> SetAttribute ( "price", os.str ( ).c_str ( ) );
			os.str ( ss :: string ( ) );
			os << ocd.getConnectPrice ( ) / 100000.0;
			elem4 -> SetAttribute ( "connectPrice", os.str ( ).c_str ( ) );
			os.str ( ss :: string ( ) );
		}
	}
	xml.PrintOn ( os );
	return os.str ( ).c_str ( );
}

BOOL CallPathResource :: LoadHeaders ( PHTTPRequest & /*request*/ ) {
	return true;
}
