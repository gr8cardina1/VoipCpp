#ifndef __MONEYSPOOL_HPP
#define __MONEYSPOOL_HPP
#pragma interface

typedef std :: vector < Pointer < AfterTask >, __SS_ALLOCATOR < Pointer < AfterTask > > > AfterTaskVector;
class MoneySpool;

class MoneyEater : public Allocatable < __SS_ALLOCATOR > {
	MoneySpool & owner;
	PTime created;
	double usedMoney;
	int lastSec;
	virtual double getMoney ( int secs ) const = 0;
	MoneyEater ( const MoneyEater & );
	MoneyEater & operator= ( const MoneyEater & );
	virtual bool zeroCost ( ) const = 0;
public:
	double getUsedMoney ( ) const;
	double getRealMoney ( ) const;
	virtual int getRealSecs ( ) const = 0;
	double tick ( double & nextSec );
	void conditionalStop ( );
	virtual void stop ( ) = 0;
	explicit MoneyEater ( MoneySpool & o );
	Pointer < AfterTask > detach ( ) const;
	virtual ~MoneyEater ( );
	virtual bool tickable ( ) const;
};

class NTMoneyEater : public Allocatable < __SS_ALLOCATOR > {
	MoneySpool & owner;
	double usedMoney;
	void useMoney ( double m );
	NTMoneyEater ( const NTMoneyEater & );
	NTMoneyEater & operator= ( const NTMoneyEater & );
	virtual bool zeroCost ( ) const = 0;
protected:
	void setMoney ( double m );
public:
	void conditionalStop ( );
	virtual void stop ( ) = 0;
	NTMoneyEater ( MoneySpool & o );
	Pointer < AfterTask > detach ( ) const;
	virtual ~NTMoneyEater ( );
	double getUsedMoney ( ) const;
	double getRealMoney ( ) const;
};

class MoneySpool : public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	double totalMoney;
	double usedMoney;
	typedef std :: set < MoneyEater *, std :: less < MoneyEater * >, __SS_ALLOCATOR < MoneyEater * > > EatersSet;
	typedef std :: set < NTMoneyEater *, std :: less < NTMoneyEater * >,
		__SS_ALLOCATOR < NTMoneyEater * > > NTEatersSet;
	EatersSet eaters;
	NTEatersSet nteaters;
	bool credit;
	virtual void destroy ( ) = 0;
	virtual void propagateBalanceChange ( ) = 0;
	virtual Pointer < AfterTask > getAfterTask ( double realMoney ) const;
	void addEater ( MoneyEater * e );
	void addEater ( NTMoneyEater * e );
	void stop ( );
	friend MoneyEater :: MoneyEater ( MoneySpool & s );
	friend class NTMoneyEater;
protected:
	double getTotalMoney ( ) const;
	void addUsedMoney ( double m );
public:
	MoneySpool ( );
	double getMoneyRemaining ( ) const;
	double getMoneyUsed ( ) const;
	Pointer < AfterTask > delEater ( const MoneyEater * e );
	Pointer < AfterTask > delEater ( const NTMoneyEater * e );
	void tick ( );
	void setTotalMoney ( double tm );
	void setCredit ( bool cr );
	virtual ~MoneySpool ( );
};

class SimpleMoneyEater : public MoneyEater {
	double price;
	virtual double getMoney ( int secs ) const;
	bool zeroCost ( ) const;
public:
	SimpleMoneyEater ( MoneySpool & o, double p );
};

class FullMoneyEater : public MoneyEater {
	// TODO: should use PriceData instead
	double price;
	TarifInfo tarif;
	PriceData :: AmortiseMap amortise;
	TarifRound round;
	virtual double addCost ( int secs ) const;
	bool zeroCost ( ) const;
	int transformLen ( int len ) const;
	double getPrice ( ) const;
	double getMoney ( int secs ) const;
public:
	FullMoneyEater ( MoneySpool & o, double p, const TarifInfo & t, const TarifRound & r,
		const PriceData :: AmortiseMap & am );
};

class TrafficEater : public NTMoneyEater {
	double price;
	bool zeroCost ( ) const;
public:
	TrafficEater ( MoneySpool & o, double p );
	void setOctets ( unsigned octets );
	void stop ( );
};

#endif
