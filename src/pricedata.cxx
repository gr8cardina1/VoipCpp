#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "tarifinfo.hpp"
#include "tarifround.hpp"
#include "pricedata.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>

PriceData :: PriceData ( double p, const TarifRound & r, const TarifInfo * t, const IntIntMap * am, int connectPrice ) :
	price ( p ), tarif ( * t ), round ( r ) {
	if ( connectPrice )
		amortise [ 1 ] = connectPrice / 100000.0;
	if ( ! am )
		return;
	for ( IntIntMap :: const_iterator i = am -> begin ( ); i != am -> end ( ); i ++ )
		amortise [ i -> first * 60 ] += i -> second / 100000.0;
}

PriceData :: PriceData ( ) : price ( 0 ) { }

double PriceData :: getPrice ( ) const {
	return price;
}

const TarifRound & PriceData :: getRound ( ) const {
	return round;
}

const TarifInfo & PriceData :: getTarif ( ) const {
	return tarif;
}

const PriceData :: AmortiseMap & PriceData :: getAmortise ( ) const {
	return amortise;
}

double getAmortisedMoney ( const PriceData :: AmortiseMap & amortise, int secs ) {
	double r = 0;
	for ( PriceData :: AmortiseMap :: const_iterator i = amortise.begin ( );
		i != amortise.end ( ) && secs >= i -> first; ++ i )
		r += i -> second;
	return r;
}

double PriceData :: getConnectMoney ( int seconds ) const {
	seconds = int ( tarif.getMinutes ( seconds ) * 60 + 0.5 );
	return getAmortisedMoney ( amortise, seconds );
}

double PriceData :: getMoney ( int seconds ) const {
	seconds = int ( tarif.getMinutes ( seconds ) * 60 + 0.5 );
	double r = 0;
	for ( AmortiseMap :: const_iterator i = amortise.begin ( );
		i != amortise.end ( ) && seconds >= i -> first; ++ i )
		r += i -> second;
	seconds = round.roundNoFree ( seconds );
	return r + price * seconds / 60.0;
}

double PriceData :: getRealMoney ( int seconds ) const {
	double r = 0;
	for ( AmortiseMap :: const_iterator i = amortise.begin ( );
		i != amortise.end ( ) && seconds >= i -> first; ++ i )
		r += i -> second;
	seconds = round.roundNoFree ( seconds );
	return r + price * seconds / 60.0;
}

int PriceData :: roundSeconds ( int seconds ) const {
	seconds = conf -> getRounded ( tarif.getMinutes ( seconds ) * 60 );
	return round.round ( seconds );
}

static double getMoney ( const PriceData :: SecondsVector & prices, int secs ) {
	double r = 0;
	for ( PriceData :: SecondsVector :: const_iterator i = prices.begin ( ); i != prices.end ( ); i ++ )
		r += i -> first -> getMoney ( i -> second + secs );
	return r;
}

static double getRealMoney ( const PriceData :: SecondsVector & prices, int secs ) {
	double r = 0;
	for ( PriceData :: SecondsVector :: const_iterator i = prices.begin ( ); i != prices.end ( ); i ++ )
		r += i -> first -> getRealMoney ( i -> second + secs );
	return r;
}

static double maximalnodopustimayaoshibka = 0.0001;

int PriceData :: getSeconds ( const SecondsVector & prices, double money ) {
	money += maximalnodopustimayaoshibka;
	int first = 1000000, len = 1000000;
	while ( len > 0 ) {
		int half = len >> 1;
		if ( :: getMoney ( prices, first - half ) > money ) {
			first -= half + 1;
			len -= half + 1;
		} else
			len = half;
	}
	return first;
	
}

int PriceData :: getRealSeconds ( const SecondsVector & prices, double money ) {
	double money1 = money + maximalnodopustimayaoshibka;
	int first = 1000000, len = 1000000;
	while ( len > 0 ) {
		int half = len >> 1;
		if ( :: getRealMoney ( prices, first - half ) > money1 ) {
			first -= half + 1;
			len -= half + 1;
		} else
			len = half;
	}
	return first;
}
