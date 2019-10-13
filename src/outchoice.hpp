#ifndef __OUTCHOICE_HPP
#define __OUTCHOICE_HPP
#pragma interface
#include "prototype.hpp"

class PUDPSocket;
class OutChoice {
	ss :: string ip, prefix, code;
	ss :: string realDigits;
	ss :: string sentDigits;
	ss :: string mgcpGw, mgcpEp;
	IntVector interfaces;
	OutCardDetails ocd;
	PUDPSocket * sock;
	int price, connectPrice, prio;
	int depth;
	int pid, port, lim;
	int numberPrio;
	int timeout;
	int tarif;
	SigOptionsPeer sigOptions;
	TarifRound round;
	bool directRTP;
	bool fromNat;
	ProtoType type;
public:
	OutChoice ( ) { }
	OutChoice ( int price_, int cp, int prio_, const ss :: string & i, int po, const ss :: string & pre, int dep,
		int peer, const ss :: string & code, int l, bool dir, const IntVector & ifs, const SigOptionsPeer & so,
		int np, const OutCardDetails & o, int to, bool fn, PUDPSocket * s, ProtoType typ, int t, const TarifRound & r,
		const ss :: string & rdig, const ss :: string & sdig, const ss :: string & mg, const ss :: string & me );
	int getPrice ( ) const;
	int getConnectPrice ( ) const;
	int getPrio ( ) const;
	const ss :: string & getIp ( ) const {
		return ip;
	}
	int getPort ( ) const {
		return port;
	}
	const ss :: string & getPrefix ( ) const {
		return prefix;
	}
	const ss :: string & getCode ( ) const {
		return code;
	}
	int getDepth ( ) const;
	int getMinusDepth ( ) const;
	int getPid ( ) const {
		return pid;
	}
	int getLim ( ) const;
	bool getDirectRTP ( ) const;
	int getNumberPrio ( ) const {
		return numberPrio;
	}
	const IntVector & getInterfaces ( ) const;
	const SigOptionsPeer & getSigOptions ( ) const;
	const OutCardDetails & getOutCardDetails ( ) const;
	int getTimeout ( ) const;
	bool getFromNat ( ) const;
	PUDPSocket * getSocket ( ) const;
	ProtoType getType ( ) const;
	int getTarif ( ) const;
	const TarifRound & getRound ( ) const;
	const ss :: string & getRealDigits ( ) const;
	const ss :: string & getSentDigits ( ) const;
	const ss :: string & getMgcpGw ( ) const;
	const ss :: string & getMgcpEp ( ) const;
};

typedef std :: multiset < OutChoice, std :: less < OutChoice >, __SS_ALLOCATOR < OutChoice > > OutChoiceSet;
typedef std :: vector < OutChoiceSet, __SS_ALLOCATOR < OutChoiceSet > > OutChoiceSetVector;
bool operator< ( const OutChoice & c1, const OutChoice & c2 );

std :: ostream & operator<< ( std :: ostream & os, const OutChoice & c );

#endif
