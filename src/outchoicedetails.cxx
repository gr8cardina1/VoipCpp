#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "signallingoptions.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "outchoice.hpp"
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <algorithm>

OutTry :: OutTry ( int ch, int ca, const ss :: string & ci ) : choiceIndex ( ch ), cause ( ca ), callId ( ci ) { }

OutChoiceDetails :: OutChoiceDetails ( const OutChoice & o, const ss :: string & cd ) : digits ( o.getPrefix ( ) ),
	ip ( o.getIp ( ) ),
	interfaces ( o.getInterfaces ( ) ), sigOptions ( o.getSigOptions ( ) ), ocd ( o.getOutCardDetails ( ) ),
	timeout ( o.getTimeout ( ) ), depth ( o.getDepth ( ) ), sock ( o.getSocket ( ) ), port ( o.getPort ( ) ),
	peer ( o.getPid ( ) ), lim ( o.getLim ( ) ), price ( o.getPrice ( ) ), connectPrice ( o.getConnectPrice ( ) ),
	tarif ( o.getTarif ( ) ), code ( o.getCode ( ) ), callingDigits ( cd ), realDigits ( o.getRealDigits ( ) ),
	sentDigits ( o.getSentDigits ( ) ), mgcpGw ( o.getMgcpGw ( ) ), mgcpEp ( o.getMgcpEp ( ) ), round ( o.getRound ( ) ),
	directRTP ( o.getDirectRTP ( ) ), fromNat ( o.getFromNat ( ) ), type ( o.getType ( ) ) { }

const ss :: string & OutChoiceDetails :: getDigits ( ) const {
	return digits;
}

const ss :: string & OutChoiceDetails :: getIp ( ) const {
	return ip;
}

int OutChoiceDetails :: getPort ( ) const {
	return port;
}

int OutChoiceDetails :: getPeer ( ) const {
	return peer;
}

int OutChoiceDetails :: getLim ( ) const {
	return lim;
}

bool OutChoiceDetails :: getDirectRTP ( ) const {
	return directRTP;
}

const IntVector & OutChoiceDetails :: getInterfaces ( ) const {
	return interfaces;
}

const SigOptionsPeer & OutChoiceDetails :: getSigOptions ( ) const {
	return sigOptions;
}

const OutCardDetails & OutChoiceDetails :: getOutCardDetails ( ) const {
	return ocd;
}

int OutChoiceDetails :: getTimeout ( ) const {
	return timeout;
}

bool OutChoiceDetails :: getFromNat ( ) const {
	return fromNat;
}

int OutChoiceDetails :: getDepth ( ) const {
	return depth;
}

PUDPSocket * OutChoiceDetails :: getSocket ( ) const {
	return sock;
}

ProtoType OutChoiceDetails :: getType ( ) const {
	return type;
}

int OutChoiceDetails :: getPrice ( ) const {
	return price;
}

int OutChoiceDetails :: getConnectPrice ( ) const {
	return connectPrice;
}

int OutChoiceDetails :: getTarif ( ) const {
	return tarif;
}

const TarifRound & OutChoiceDetails :: getRound ( ) const {
	return round;
}

const ss :: string & OutChoiceDetails :: getCode ( ) const {
	return code;
}

const ss :: string & OutChoiceDetails :: getCallingDigits ( ) const {
	return callingDigits;
}

const ss :: string & OutChoiceDetails :: getRealDigits ( ) const {
	return realDigits;
}

const ss :: string & OutChoiceDetails :: getSentDigits ( ) const {
	return sentDigits;
}

const ss :: string & OutChoiceDetails :: getMgcpGw ( ) const {
	return mgcpGw;
}

const ss :: string & OutChoiceDetails :: getMgcpEp ( ) const {
	return mgcpEp;
}

std :: ostream & operator<< ( std :: ostream & os, const OutChoiceDetails & c ) {
	return os << "peer: " << c.getPeer ( ) << ", ip: " << c.getIp ( ) << ", port: " << c.getPort ( )
		<< ", fromNat: " << c.getFromNat ( ) << ", digits: " << c.getDigits ( )
		<< ", realDigits: " << c.getRealDigits ( ) << ", sentDigits: " << c.getSentDigits ( )
		<< ", type: " << int ( c.getType ( ) );
}

bool hasH323Choices ( const OutChoiceDetailsVector & choices ) {
	return std :: find_if ( choices.begin ( ), choices.end ( ),
		bind ( & OutChoiceDetails :: getType, boost :: lambda :: _1 ) == ptH323 ) != choices.end ( );
}

bool hasSipChoices ( const OutChoiceDetailsVector & choices ) {
	using boost :: lambda :: _1;
	return std :: find_if ( choices.begin ( ), choices.end ( ),
		bind ( & OutChoiceDetails :: getType, _1 ) == ptSip ) != choices.end ( );
}

