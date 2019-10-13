#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "signallingoptions.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"

OutCardDetails :: OutCardDetails ( ) { }

OutCardDetails :: OutCardDetails ( const ss :: string & d, int p ) : digits ( d ), price ( p ), connectPrice ( 0 ),
	euPrice ( 0 ), euConnectPrice ( 0 ), tarif ( 0 ), euTarif ( 0 ), redirect ( false ) { }

OutCardDetails :: OutCardDetails ( const ss :: string & d, const ss :: string & c, const ss :: string & ec, bool r,
	int p, int cp, int ep, int ecp, int t, int et, const TarifRound & ro, const TarifRound & er,
	const IntIntMap & am ) : digits ( d ), code ( c ), euCode ( ec ), price ( p ),
	connectPrice ( cp ), euPrice ( ep ), euConnectPrice ( ecp ), tarif ( t ), euTarif ( et ), round ( ro ),
	euRound ( er ), redirect ( r ) {
	for ( IntIntMap :: const_iterator i = am.begin ( ); i != am.end ( ); i ++ )
		amortise [ i -> first * 60 ] += i -> second / 100000.0;
}

const ss :: string & OutCardDetails :: getDigits ( ) const {
	return digits;
}

const ss :: string & OutCardDetails :: getCode ( ) const {
	return code;
}

const ss :: string & OutCardDetails :: getEuCode ( ) const {
	return euCode;
}

int OutCardDetails :: getPrice ( ) const {
	return price;
}

int OutCardDetails :: getConnectPrice ( ) const {
	return connectPrice;
}

int OutCardDetails :: getEuPrice ( ) const {
	return euPrice;
}

int OutCardDetails :: getEuConnectPrice ( ) const {
	return euConnectPrice;
}

int OutCardDetails :: getTarif ( ) const {
	return tarif;
}

int OutCardDetails :: getEuTarif ( ) const {
	return euTarif;
}

const TarifRound & OutCardDetails :: getRound ( ) const {
	return round;
}

const TarifRound & OutCardDetails :: getEuRound ( ) const {
	return euRound;
}

bool OutCardDetails :: getRedirect ( ) const {
	return redirect;
}

const PriceData :: AmortiseMap & OutCardDetails :: getAmortise ( ) const {
	return amortise;
}
