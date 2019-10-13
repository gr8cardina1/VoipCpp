#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "tarifround.hpp"
#include "priceelement.hpp"
#include <stdexcept>
#include "unknowndigit.hpp"
#include "getindex.hpp"
#include "pointer.hpp"

PriceElement :: PriceElement ( ) : price ( 0 ), connectPrice ( 0 ), enabled ( true ), exists ( false ),
	prio ( - 1 ), minDigits ( 0 ), maxDigits ( 0 ) {
	for ( int i = 0; i < 12; i ++ )
		children [ i ] = 0;
}

PriceElement :: PriceElement ( const PriceElement * p ) : price ( p -> price ),
	connectPrice ( p -> connectPrice ), round ( p -> round ), enabled ( p -> enabled ), exists ( false ),
	prio ( p -> prio ), minDigits ( p -> minDigits ), maxDigits ( p -> maxDigits ) {
	for ( int i = 0; i < 12; i ++ )
		children [ i ] = 0;
}

void PriceElement :: setAt ( char c, PriceElement * child ) {
	try {
		children [ getIndex ( c ) ] = child;
	} catch ( ... ) {
		delete child;
		throw;
	}
}

PriceElement * PriceElement :: getAt ( char c ) const {
	return children [ getIndex ( c ) ];
}

int PriceElement :: getPrice ( ) const {
	return price;
}

int PriceElement :: getConnectPrice ( ) const {
	return connectPrice;
}

void PriceElement :: setPrice ( int p, int cp ) {
	price = p;
	connectPrice = cp;
}

PriceElement :: ~PriceElement ( ) {
	for ( int i = 0; i < 12; i ++ )
		safeDel ( children [ i ] );
}

const TarifRound & PriceElement :: getRound ( ) const {
	return round;
}

void PriceElement :: setRound ( const TarifRound & r ) {
	round = r;
}

bool PriceElement :: getEnabled ( ) const {
	return enabled;
}

void PriceElement :: setEnabled ( bool en ) {
	enabled = en;
}

bool PriceElement :: getExists ( ) const {
	return exists;
}

void PriceElement :: setExists ( bool ex ) {
	exists = ex;
}

char PriceElement :: getPrio ( ) const {
	return prio;
}

void PriceElement :: setPrio ( char pr ) {
	prio = pr;
}

void PriceElement :: setDigits ( unsigned char mi, unsigned char ma ) {
	minDigits = mi;
	maxDigits = ma;
}

unsigned char PriceElement :: getMinDigits ( ) const {
	return minDigits;
}

unsigned char PriceElement :: getMaxDigits ( ) const {
	return maxDigits;
}
