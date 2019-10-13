#pragma implementation
#include "ntptimestamp.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

unsigned long long ntpTimestamp ( ) {
	boost :: posix_time :: ptime t = boost :: posix_time :: microsec_clock :: local_time ( );
	boost :: posix_time :: ptime o ( boost :: gregorian :: date ( 1900, boost :: gregorian :: Jan, 1 ) );
	boost :: posix_time :: time_duration d = t - o;
	unsigned long seconds = d.total_seconds ( ); //negative
	unsigned long long r = seconds;
	r <<= 32;
	unsigned long long frac = d.fractional_seconds ( );
	frac <<= 32;
	frac /= boost :: posix_time :: time_duration :: ticks_per_second ( );
	r |= frac;
	return r;
}
