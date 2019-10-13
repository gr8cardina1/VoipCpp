#ifndef _PROFITGUARD_HPP_
#define _PROFITGUARD_HPP_
#pragma interface

class ProfitGuard {
public:
	ProfitGuard ( const char * * po, double valuteRate );
	ProfitGuard ( ) : minProfitRel ( 0 ), minProfitAbs ( 0 ), enabled ( false ) { }
	virtual ~ProfitGuard ( );
	bool isProfitable ( int inPrice, int outPrice ) const;
	int getMinProfitAbs ( ) const { return minProfitAbs; }
	double getMinProfitRel ( ) const { return minProfitRel; }
	bool isEnabled ( ) const { return enabled; }
private:
	double minProfitRel;
	int minProfitAbs;
	bool enabled;
};

#endif //_PROFITGUARD_HPP_
