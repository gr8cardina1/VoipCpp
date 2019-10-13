#pragma implementation "tarifround.hpp"
#include "tarifround.hpp"

TarifRound :: TarifRound ( int rm, int re, int rf ) {
	if ( rm < 0 )
		rm = 0;
	if ( re < 1 )
		re = 1;
	if ( rf < 0 )
		rf = 0;
	rmin = char ( rm );
	reach = char ( re );
	rfree = char ( rf );
}
int TarifRound :: round ( int secs ) const {
	if ( secs <= int ( rfree ) )
		return 0;
	if ( secs <= int ( rmin ) )
		return rmin;
	secs += reach - 1;
	secs /= reach;
	secs *= reach;
	return secs;
}
int TarifRound :: roundNoFree ( int secs ) const {
	if ( secs <= int ( rmin ) )
		return rmin;
	secs += reach - 1;
	secs /= reach;
	secs *= reach;
	return secs;
}
int TarifRound :: roundDown ( int secs ) const {
	if ( secs <= int ( rfree ) )
		return 0;
	if ( secs < int ( rmin ) )
		return 0;
	secs /= reach;
	secs *= reach;
	return secs;
}
