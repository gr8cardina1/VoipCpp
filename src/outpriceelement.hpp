#ifndef __OUTPRICEELEMENT_HPP
#define __OUTPRICEELEMENT_HPP
#pragma interface

struct OutChoiceInt {
	int pid:31;
	bool enabled:1;
	int price;
	int connectPrice;
	char prio;
	TarifRound round;
	OutChoiceInt ( int pi, int pri, int cp, bool en, char p, const TarifRound & r );
};
bool operator< ( const OutChoiceInt & c1, const OutChoiceInt & c2 );
class OutPriceElement : public Allocatable < __SS_ALLOCATOR > {
public:
	typedef std :: multiset < OutChoiceInt, std :: less < OutChoiceInt >,
		__SS_ALLOCATOR < OutChoiceInt > > ChoicesSet;
private:
	OutPriceElement * children [ 12 ];
	ChoicesSet choices;
	void clear ( );
	OutPriceElement ( const OutPriceElement & );
	OutPriceElement & operator= ( const OutPriceElement & );
public:
	OutPriceElement ( );
	~OutPriceElement ( );
	void setAt ( char c, OutPriceElement * child );
	OutPriceElement * getAt ( char c ) const;
	void append ( const OutChoiceInt & o );
	const ChoicesSet & getChoices ( ) const;
	bool hasPeer ( int peer ) const;
};
#endif
