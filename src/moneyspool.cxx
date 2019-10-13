#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include "tarifinfo.hpp"
#include "tarifround.hpp"
#include "pricedata.hpp"
#include "aftertask.hpp"
#include <ptlib.h>
#include "moneyspool.hpp"
#include <ptlib/sockets.h>
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <stdexcept>
#include <cmath>
#include <algorithm>

MoneySpool :: MoneySpool ( ) : totalMoney ( 0 ), usedMoney ( 0 ), credit ( false ) { }

double MoneySpool :: getMoneyRemaining ( ) const {
	return totalMoney - usedMoney;
}

double MoneySpool :: getMoneyUsed ( ) const {
	return usedMoney;
}

void MoneySpool :: addEater ( MoneyEater * e ) {
	eaters.insert ( e );
}

void MoneySpool :: addEater ( NTMoneyEater * e ) {
	nteaters.insert ( e );
}

Pointer < AfterTask > MoneySpool :: getAfterTask ( double /*realMoney*/ ) const {
	return 0;
}

double MoneySpool :: getTotalMoney ( ) const {
	return totalMoney;
}

Pointer < AfterTask > MoneySpool :: delEater ( const MoneyEater * e ) {
	if ( ! eaters.count ( const_cast < MoneyEater * > ( e ) ) )
		throw std :: runtime_error ( "double delEater" );
	double realMoney = e -> getRealMoney ( );
	totalMoney -= realMoney;
	propagateBalanceChange ( );
	usedMoney -= e -> getUsedMoney ( );
	eaters.erase ( const_cast < MoneyEater * > ( e ) );
	Pointer < AfterTask > a = getAfterTask ( realMoney );
	if ( eaters.empty ( ) && nteaters.empty ( ) )
		destroy ( );
	return a;
}

Pointer < AfterTask > MoneySpool :: delEater ( const NTMoneyEater * e ) {
	if ( ! nteaters.count ( const_cast < NTMoneyEater * > ( e ) ) )
		throw std :: runtime_error ( "double delEater" );
	double realMoney = e -> getRealMoney ( );
	totalMoney -= realMoney;
	propagateBalanceChange ( );
	usedMoney -= e -> getUsedMoney ( );
	nteaters.erase ( const_cast < NTMoneyEater * > ( e ) );
	Pointer < AfterTask > a = getAfterTask ( realMoney );
	if ( eaters.empty ( ) && nteaters.empty ( ) )
		destroy ( );
	return a;
}

void MoneySpool :: addUsedMoney ( double m ) {
	usedMoney += m;
	if ( ! credit && usedMoney >= totalMoney )
		stop ( );
}

void MoneySpool :: tick ( ) {
	double nextSecTotal = 0;
	for ( EatersSet :: const_iterator i = eaters.begin ( ); i != eaters.end ( ); ++ i ) {
		if ( ! ( * i ) -> tickable ( ) )
			continue;
		double nextSecCur = 0;
		usedMoney += ( * i ) -> tick ( nextSecCur );
		nextSecTotal += nextSecCur;
	}
	if ( ! credit && /*usedMoney +*/ nextSecTotal > totalMoney )
		stop ( );
}

void MoneySpool :: stop ( ) {
	std :: for_each ( eaters.begin ( ), eaters.end ( ), std :: mem_fun ( & MoneyEater :: conditionalStop ) );
	std :: for_each ( nteaters.begin ( ), nteaters.end ( ), std :: mem_fun ( & NTMoneyEater :: conditionalStop ) );
}

void MoneySpool :: setTotalMoney ( double tm ) {
	totalMoney = tm;
}

void MoneySpool :: setCredit ( bool cr ) {
	credit = cr;
}

MoneySpool :: ~MoneySpool ( ) { }

void NTMoneyEater :: conditionalStop ( ) {
	if ( ! zeroCost ( ) )
		stop ( );
}

double NTMoneyEater :: getUsedMoney ( ) const {
	return usedMoney;
}

double NTMoneyEater :: getRealMoney ( ) const {
	return usedMoney;
}

void NTMoneyEater :: useMoney ( double m ) {
	owner.addUsedMoney ( m );
}

void NTMoneyEater :: setMoney ( double m ) {
	useMoney ( m - usedMoney );
	usedMoney = m;
}

NTMoneyEater :: NTMoneyEater ( MoneySpool & o ) : owner ( o ), usedMoney ( 0 ) {
	owner.addEater ( this );
}

NTMoneyEater :: ~NTMoneyEater ( ) { }

Pointer < AfterTask > NTMoneyEater :: detach ( ) const {
	return owner.delEater ( this );
}

void MoneyEater :: conditionalStop ( ) {
	if ( ! zeroCost ( ) )
		stop ( );
}

double MoneyEater :: getUsedMoney ( ) const {
	return usedMoney;
}

double MoneyEater :: getRealMoney ( ) const {
	return getMoney ( getRealSecs ( ) );
}

bool MoneyEater :: tickable ( ) const {
	return true;
}

double MoneyEater :: tick ( double & nextSec ) {
	PTime now;
	int curSec = conf -> getRounded ( double ( ( now - created ).GetMilliSeconds ( ) ) / 1000.0 );
	double curMoney = getMoney ( curSec );
	nextSec = getMoney ( curSec + 1 ) - curMoney;
	lastSec = curSec;
	double tickMoney = curMoney - usedMoney;
	usedMoney = curMoney;
	return tickMoney;
}

MoneyEater :: MoneyEater ( MoneySpool & o ) : owner ( o ), usedMoney ( 0 ), lastSec ( 0 ) {
	owner.addEater ( this );
}

MoneyEater :: ~MoneyEater ( ) { }

Pointer < AfterTask > MoneyEater :: detach ( ) const {
	return owner.delEater ( this );
}

SimpleMoneyEater :: SimpleMoneyEater ( MoneySpool & o, double p ) : MoneyEater ( o ), price ( p / 60 ) { }

bool SimpleMoneyEater :: zeroCost ( ) const {
	return std :: abs ( price ) < 0.000000001;
}

double SimpleMoneyEater :: getMoney ( int secs ) const {
	return secs * price;
}

FullMoneyEater :: FullMoneyEater ( MoneySpool & o, double p, const TarifInfo & t, const TarifRound & r,
	const PriceData :: AmortiseMap & am ) : MoneyEater ( o ), price ( p / 60 ), tarif ( t ), amortise ( am ),
	round ( r ) { }

bool FullMoneyEater :: zeroCost ( ) const {
	return std :: abs ( price ) < 0.000000001 && amortise.empty ( );
}

int FullMoneyEater :: transformLen ( int len ) const {
	len = int ( tarif.getMinutes ( len ) * 60 + 0.5 );
	len = round.round ( len );
	return len;
}

double FullMoneyEater :: getPrice ( ) const {
	return price;
}

double FullMoneyEater :: getMoney ( int secs ) const {
	return transformLen ( secs ) * price + addCost ( secs );
}

double FullMoneyEater :: addCost ( int secs ) const {
	double c = 0;
	for ( PriceData :: AmortiseMap :: const_iterator i = amortise.begin ( );
		i != amortise.end ( ) && secs >= i -> first; ++ i )
		c += i -> second;
	return c;
}

TrafficEater :: TrafficEater ( MoneySpool & o, double p ) : NTMoneyEater ( o ), price ( p ) { }

bool TrafficEater :: zeroCost ( ) const {
	return std :: abs ( price ) < 0.000000001;
}

void TrafficEater :: setOctets ( unsigned octets ) {
	setMoney ( price * octets );
}

void TrafficEater :: stop ( ) { } //can't stop for now
