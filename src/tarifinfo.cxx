#pragma implementation "tarifinfo.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include "tarifinfo.hpp"

int TarifInfo :: getSeconds ( double mins ) const {
	int spm = 60;
	int secs = 0;
	int min = 0;
	for ( IntIntMap :: const_iterator i = minsToSecs.begin ( ); i != minsToSecs.end ( ) && mins > i -> first; ++ i ) {
		secs += ( i -> first - min ) * spm;
		spm = i -> second;
		min = i -> first;
	}
	return secs + int ( ( mins - min ) * spm );
}
double TarifInfo :: getMinutes ( int seconds ) const {
	int spm = 60;
	int mins = 0;
	for ( IntIntMap :: const_iterator i = minsToSecs.begin ( ); i != minsToSecs.end ( ) &&
		seconds > ( i -> first - mins ) * spm; ++ i ) {
		seconds -= ( i -> first - mins ) * spm;
		mins = i -> first;
		spm = i -> second;
	}
	return mins + double ( seconds ) / spm;
}

