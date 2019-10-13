#pragma implementation
#include "ss.hpp"
#include <queue>
#include <ptlib.h>
#include "geganet.hpp"
Geganet :: Geganet ( ) : nActive ( 0 ) { }
bool Geganet :: take ( int timeout, int nChannels ) {
	if ( nActive >= nChannels )
		return false;
	QueueType :: size_type rcSize = rcs.size ( );
	if ( rcSize + nActive < QueueType :: size_type ( nChannels ) ) {
		nActive ++;
		return true;
	}
	while ( rcSize + nActive > QueueType :: size_type( nChannels ) ) {
		rcSize --;
		rcs.pop ( );
	}
	if ( ( PTime ( ) - rcs.front ( ) ).GetSeconds ( ) < timeout )
		return false;
	nActive ++;
	return true;
}
void Geganet :: release ( bool connected ) {
	if ( nActive > 0 )
		nActive --;
	if ( connected )
		rcs.push ( PTime ( ) );
}
