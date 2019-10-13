#ifndef _LATENCYLIMITS_HPP_
#define _LATENCYLIMITS_HPP_
#pragma interface

struct LatencyLimits {
	long minConnectTime;
	long minAlertingTime;
	long minCallProceedingTime;
	long minCallDuration;
	bool enabled;
	LatencyLimits ( ) : minConnectTime ( 0 ), minAlertingTime ( 0 ), minCallProceedingTime ( 0 ),
		minCallDuration ( 0 ), enabled ( false ) { }
};

#endif
