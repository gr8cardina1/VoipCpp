#pragma implementation
#include "profitguard.hpp"
#include <ptlib.h>
#include <ptlib/svcproc.h>
#include <cstdlib>

ProfitGuard :: ProfitGuard ( const char * * po, double valuteRate ) {
	union ProfitOptions {
		const char * p [ 4 ];
		struct {
			const char * id;
			const char * name;
			const char * minProfitAbs;
			const char * minProfitRel;
		} n;
	};
	ProfitOptions * p = reinterpret_cast < ProfitOptions * > ( po );
	double t = std :: atof ( p -> n.minProfitAbs ) / valuteRate * 100000;
	if ( t < 0 )
		t -= 0.5;
	else
		t += 0.5;
	minProfitAbs = int ( t );
	minProfitRel = std :: atof ( p -> n.minProfitRel );
	enabled = true;
}

ProfitGuard :: ~ProfitGuard ( ) {
}

bool ProfitGuard :: isProfitable ( int inPrice, int outPrice ) const {
	if ( ! enabled ) return true;
	int profit = inPrice - outPrice;
	double rel = ( (double) profit / inPrice ) * 100;
	if ( ( profit < minProfitAbs ) || ( rel < minProfitRel ) )
		return false;
	return true;
}
