#pragma implementation
#include "tarifround.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include "outpriceelement.hpp"
#include <algorithm>
#include <stdexcept>
#include "unknowndigit.hpp"
#include "getindex.hpp"
#include "pointer.hpp"

OutChoiceInt :: OutChoiceInt ( int pi, int pri, int cp, bool en, char p, const TarifRound & r ) : pid ( pi ),
	enabled ( en ), price ( pri ), connectPrice ( cp ), prio ( p ), round ( r ) { }

bool operator < ( const OutChoiceInt & c1, const OutChoiceInt & c2 ) {
	return c1.price < c2.price;
}

struct peerEq {
	int pid;
	peerEq ( int p ) : pid ( p ) { }
	bool operator() ( const OutChoiceInt & o ) {
		return o.pid == pid;
	}
};

void OutPriceElement :: setAt ( char c, OutPriceElement * child ) {
	try {
		children [ getIndex ( c ) ] = child;
	} catch ( ... ) {
		delete child;
		throw;
	}
}

OutPriceElement * OutPriceElement :: getAt ( char c ) const {
	return children [ getIndex ( c ) ];
}

void OutPriceElement :: append ( const OutChoiceInt & o ) {
	choices.insert ( o );
}

const OutPriceElement :: ChoicesSet & OutPriceElement :: getChoices ( ) const {
	return choices;
}

OutPriceElement :: OutPriceElement ( ) {
	for ( int i = 0; i < 12; i ++ )
		children [ i ] = 0;
}

OutPriceElement :: ~OutPriceElement ( ) {
	for ( int i = 0; i < 12; i ++ )
		safeDel ( children [ i ] );
}

void OutPriceElement :: clear ( ) {
	choices.clear ( );
	for ( int i = 0; i < 12; i ++ )
		if ( children [ i ] )
			children [ i ] -> clear ( );
}

bool OutPriceElement :: hasPeer ( int peer ) const {
	return count_if ( choices.begin ( ), choices.end ( ), peerEq ( peer ) );
}
