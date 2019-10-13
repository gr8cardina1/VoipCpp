#ifndef __SOURCEDATA_HPP
#define __SOURCEDATA_HPP
#pragma interface

struct SourceData {
	double valuteRate;
	PriceData :: AmortiseMap amortise;
	ss :: string name, rname;
	ss :: string acctn, outAcctn, code, euCode;
	ss :: string ip;
	ss :: string inPrefix;
	int peer;
	int price, connectPrice, euPrice, euConnectPrice;
	int tarif, euTarif;
	TarifRound round, euRound;
	bool inLocked, outLocked;
	enum Type { //serialization fails on anonymous enums
		unknown,
		inbound,
		card
	} type;
	bool allPrice;
	SourceData ( ) : valuteRate ( 1 ), peer ( 0 ), price ( 0 ), connectPrice ( 0 ), euPrice ( 0 ), euConnectPrice ( 0 ),
		tarif ( 0 ), euTarif ( 0 ), inLocked ( false ), outLocked ( false ), type ( unknown ), allPrice ( false ) { }
};
namespace boost {
namespace serialization {

template < class Archive > void serialize ( Archive & ar, SourceData & g, const unsigned int /*version*/ ) {
	ar & g.inLocked;
	ar & g.outLocked;
	ar & g.type;
	ar & g.peer;
	ar & g.acctn;
	ar & g.outAcctn;
	ar & g.code;
	ar & g.euCode;
	ar & g.price;
	ar & g.connectPrice;
	ar & g.euPrice;
	ar & g.euConnectPrice;
	ar & g.tarif;
	ar & g.euTarif;
	ar & g.round;
	ar & g.euRound;
	ar & g.ip;
	ar & g.valuteRate;
	ar & g.inPrefix;
	ar & g.amortise;
}

}
}
#endif
