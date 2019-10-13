#ifndef __OUTCHOICEDETAILS_HPP
#define __OUTCHOICEDETAILS_HPP
#pragma interface
#include "prototype.hpp"
struct OutTry {
	int choiceIndex, cause;
	ss :: string callId;
	OutTry ( int ch, int ca, const ss :: string & ci );
private:
	friend class boost :: serialization :: access;
	OutTry ( ) : choiceIndex ( 0 ), cause ( 0 ) { }
	template < class Archive > void serialize ( Archive & ar, const unsigned int /*version*/ ) {
		ar & choiceIndex;
		ar & cause;
		ar & callId;
	}
};

class OutChoice;
class PUDPSocket;
class OutCardDetails;

class OutChoiceDetails {
	friend class boost :: serialization :: access;
	ss :: string digits;
	ss :: string ip;
	IntVector interfaces;
	SigOptionsPeer sigOptions;
	OutCardDetails ocd;
	int timeout;
	int depth;
	PUDPSocket * sock;
	int port;
	int peer;
	int lim;
	int price, connectPrice, tarif;
	ss :: string code;
	ss :: string callingDigits;
	ss :: string realDigits;
	ss :: string sentDigits;
	ss :: string mgcpGw;
	ss :: string mgcpEp;
	TarifRound round;
	bool directRTP;
	bool fromNat;
	ProtoType type;
	template < class Archive > void serialize ( Archive & ar, const unsigned int /*version*/ ) {
		ar & digits;
		ar & ip;
		ar & port;
		ar & peer;
		ar & lim;
		ar & directRTP;
		ar & fromNat;
//		ar & interfaces;
//		ar & sigOptions;
		ar & ocd;
		ar & timeout;
		ar & depth;
//		ar & sock;
		ar & type;
		ar & price;
		ar & connectPrice;
		ar & tarif;
		ar & round;
		ar & code;
		ar & callingDigits;
		ar & realDigits;
	}
	OutChoiceDetails ( ) : sock ( 0 ) { }
public:
//	OutChoiceDetails ( const ss :: string & d, const ss :: string & i, int po, int pe, int l, bool dir,
//		const IntVector & ifs, const SigOptionsPeer & so, const OutCardDetails & c, int to,
//		bool fn, int de, PUDPSocket * s, ProtoType typ, int pr, int cp, int t,
//		const TarifRound & r, const ss :: string & code, const ss :: string & cd, const ss :: string & rd,
//		const ss :: string & sd, const ss :: string & mg, const ss :: string & me );
	OutChoiceDetails ( const OutChoice & o, const ss :: string & cd );
	const ss :: string & getDigits ( ) const;
	const ss :: string & getRealDigits ( ) const;
	const ss :: string & getIp ( ) const;
	int getPort ( ) const;
	int getPeer ( ) const;
	int getLim ( ) const;
	bool getDirectRTP ( ) const;
	const IntVector & getInterfaces ( ) const;
	const SigOptionsPeer & getSigOptions ( ) const;
	const OutCardDetails & getOutCardDetails ( ) const;
	int getTimeout ( ) const;
	bool getFromNat ( ) const;
	int getDepth ( ) const;
	PUDPSocket * getSocket ( ) const;
	ProtoType getType ( ) const;
	int getPrice ( ) const;
	int getConnectPrice ( ) const;
	int getTarif ( ) const;
	const TarifRound & getRound ( ) const;
	const ss :: string & getCode ( ) const;
	const ss :: string & getCallingDigits ( ) const;
	const ss :: string & getSentDigits ( ) const;
	const ss :: string & getMgcpGw ( ) const;
	const ss :: string & getMgcpEp ( ) const;
};

typedef std :: vector < OutTry, __SS_ALLOCATOR < OutTry > > OutTryVector;
typedef std :: vector < OutChoiceDetails, __SS_ALLOCATOR < OutChoiceDetails > > OutChoiceDetailsVector;
typedef std :: vector < OutChoiceDetailsVector, __SS_ALLOCATOR < OutChoiceDetailsVector > > OutChoiceDetailsVectorVector;

std :: ostream & operator<< ( std :: ostream & os, const OutChoiceDetails & c );

bool hasSipChoices ( const OutChoiceDetailsVector & choices );
bool hasH323Choices ( const OutChoiceDetailsVector & choices );

#endif
