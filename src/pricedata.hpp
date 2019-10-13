#ifndef __PRICEDATA_HPP
#define __PRICEDATA_HPP
#pragma interface

class PriceData {
public:
	typedef std :: map < int, double, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, double > > > AmortiseMap;
	typedef std :: vector < std :: pair < const PriceData *, int >,
		__SS_ALLOCATOR < std :: pair < const PriceData *, int > > > SecondsVector;
private:
	double price;
	TarifInfo tarif;
	AmortiseMap amortise;
	TarifRound round;
public:
	double getConnectMoney ( int seconds ) const;
	double getMoney ( int seconds ) const;
	double getRealMoney ( int seconds ) const;
	PriceData ( double p, const TarifRound & r, const TarifInfo * t, const IntIntMap * am, int connectPrice );
	PriceData ( );
	double getPrice ( ) const;
	const TarifRound & getRound ( ) const;
	const TarifInfo & getTarif ( ) const;
	const AmortiseMap & getAmortise ( ) const;
	int roundSeconds ( int seconds ) const;

	static int getSeconds ( const SecondsVector & prices, double money );
	static int getRealSeconds ( const SecondsVector & prices, double money );
};

double getAmortisedMoney ( const PriceData :: AmortiseMap & amortise, int secs );

#endif
