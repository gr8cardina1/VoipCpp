#ifndef __OUTCARDDETAILS_HPP
#define __OUTCARDDETAILS_HPP
#pragma interface

namespace boost {
	namespace serialization {
		class access;
	}
}

class OutCardDetails {
	friend class boost :: serialization :: access;
	ss :: string digits, code, euCode;
	int price, connectPrice, euPrice, euConnectPrice;
	int tarif, euTarif;
	PriceData :: AmortiseMap amortise;
	TarifRound round, euRound;
	bool redirect;
	template < class Archive > void serialize ( Archive & ar, const unsigned int /*version*/ ) {
		ar & digits;
		ar & code;
		ar & euCode;
		ar & redirect;
		ar & price;
		ar & connectPrice;
		ar & euPrice;
		ar & euConnectPrice;
		ar & tarif;
		ar & euTarif;
		ar & round;
		ar & euRound;
		ar & amortise;
	}
public:
	OutCardDetails ( );
	OutCardDetails ( const ss :: string & d, int p );
	OutCardDetails ( const ss :: string & d, const ss :: string & c, const ss :: string & ec, bool r, int p,
		int cp, int ep, int ecp, int t, int et, const TarifRound & ro, const TarifRound & er,
		const IntIntMap & am );
	const ss :: string & getDigits ( ) const;
	const ss :: string & getCode ( ) const;
	const ss :: string & getEuCode ( ) const;
	int getPrice ( ) const;
	int getConnectPrice ( ) const;
	int getEuPrice ( ) const;
	int getEuConnectPrice ( ) const;
	int getTarif ( ) const;
	int getEuTarif ( ) const;
	const TarifRound & getRound ( ) const;
	const TarifRound & getEuRound ( ) const;
	bool getRedirect ( ) const;
	const PriceData :: AmortiseMap & getAmortise ( ) const;
};
#endif
