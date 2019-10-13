#pragma implementation
#pragma implementation "signallingoptions.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include "signallingoptions.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoice.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "priceprios.hpp"

OutChoice :: OutChoice ( int price_, int cp, int prio_, const ss :: string & i, int p, const ss :: string & pre,
	int dep, int peer, const ss :: string & cod, int l, bool dir, const IntVector & ifs,
	const SigOptionsPeer & so, int np, const OutCardDetails & o, int to, bool fn,
	PUDPSocket * s, ProtoType typ, int t, const TarifRound & r, const ss :: string & rdig, const ss :: string & sdig,
	const ss :: string & mg, const ss :: string & me ) : ip ( i ), prefix ( pre ), code ( cod ), realDigits ( rdig ),
	sentDigits ( sdig ), mgcpGw ( mg ), mgcpEp ( me ), interfaces ( ifs ), ocd ( o ), sock ( s ), price ( price_ ),
	connectPrice ( cp ), prio ( prio_ ), depth ( dep ), pid ( peer ), port ( p ), lim ( l ), numberPrio ( np ),
	timeout ( to ), tarif ( t ), sigOptions ( so ), round ( r ), directRTP ( dir ), fromNat ( fn ), type ( typ ) { }

int OutChoice :: getPrice ( ) const {
	return price;
}

int OutChoice :: getConnectPrice ( ) const {
	return connectPrice;
}

int OutChoice :: getPrio ( ) const {
	return prio;
}

int OutChoice :: getDepth ( ) const {
	return depth;
}

int OutChoice :: getMinusDepth ( ) const {
	return - depth;
}

int OutChoice :: getLim ( ) const {
	return lim;
}

bool OutChoice :: getDirectRTP ( ) const {
	return directRTP;
}

const IntVector & OutChoice :: getInterfaces ( ) const {
	return interfaces;
}

const SigOptionsPeer & OutChoice :: getSigOptions ( ) const {
	return sigOptions;
}

const OutCardDetails & OutChoice :: getOutCardDetails ( ) const {
	return ocd;
}

int OutChoice :: getTimeout ( ) const {
	return timeout;
}

bool OutChoice :: getFromNat ( ) const {
	return fromNat;
}

PUDPSocket * OutChoice :: getSocket ( ) const {
	return sock;
}

ProtoType OutChoice :: getType ( ) const {
	return type;
}

int OutChoice :: getTarif ( ) const {
	return tarif;
}

const TarifRound & OutChoice :: getRound ( ) const {
	return round;
}

bool operator< ( const OutChoice & c1, const OutChoice & c2 ) {
	long t = c1.getNumberPrio ( ) - c2.getNumberPrio ( );
	if ( t < 0 )
		return true;
	if ( t > 0 )
		return false;
	if ( c1.getPid ( ) == c2.getPid ( ) && c1.getIp ( ) == c2.getIp ( ) && c1.getPort ( ) == c2.getPort ( ) ) {
		t = c1.getCode ( ).size ( ) - c2.getCode ( ).size ( );
		if ( t > 0 )
			return true;
		if ( t < 0 )
			return false;
	}
	const PricePrios & pr = conf -> getPrios ( );
	t = ( c1.*pr.f1 ) ( ) - ( c2.*pr.f1 ) ( );
	if ( t < 0 )
		return true;
	if ( t > 0 )
		return false;
	t = ( c1.*pr.f2 ) ( ) - ( c2.*pr.f2 ) ( );
	if ( t < 0 )
		return true;
	if ( t > 0 )
		return false;
	return ( c1.*pr.f3 ) ( ) < ( c2.*pr.f3 ) ( );
}

const ss :: string & OutChoice :: getRealDigits ( ) const {
	return realDigits;
}

const ss :: string & OutChoice :: getSentDigits ( ) const {
	return sentDigits;
}

const ss :: string & OutChoice :: getMgcpGw ( ) const {
	return mgcpGw;
}

const ss :: string & OutChoice :: getMgcpEp ( ) const {
	return mgcpEp;
}

ostream & operator<< ( ostream & os, const OutChoice & c ) {
	return os << "peer: " << c.getPid ( ) << ", ip: " << c.getIp ( ) << ", digits: " <<
		c.getPrefix ( ) << ", code: " << c.getCode ( ) << ", price: " << c.getPrice ( ) <<
		", connectPrice: " << c.getConnectPrice ( ) << ", prio: " << c.getPrio ( ) << ", depth: " <<
		c.getDepth ( ) << ", realDigits: " << c.getRealDigits ( ) << ", type: " << int ( c.getType ( ) );
}
