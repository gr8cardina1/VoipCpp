#ifndef __PRICEELEMENT_HPP
#define __PRICEELEMENT_HPP
#pragma interface

class PriceElement : public Allocatable < __SS_ALLOCATOR > {
	PriceElement * children [ 12 ];
	int price, connectPrice;
	TarifRound round;
	bool enabled;
	bool exists;
	char prio;
	unsigned char minDigits, maxDigits;
	PriceElement ( const PriceElement & );
	PriceElement & operator= ( const PriceElement & );
public:
	PriceElement ( );
	PriceElement ( const PriceElement * p );
	~PriceElement ( );
	void setAt ( char c, PriceElement * child );
	PriceElement * getAt ( char c ) const;
	int getPrice ( ) const;
	int getConnectPrice ( ) const;
	void setPrice ( int p, int cp );
	const TarifRound & getRound ( ) const;
	void setRound ( const TarifRound & r );
	bool getEnabled ( ) const;
	void setEnabled ( bool en );
	bool getExists ( ) const;
	void setExists ( bool ex );
	char getPrio ( ) const;
	void setPrio ( char pr );
	void setDigits ( unsigned char mi, unsigned char ma );
	unsigned char getMinDigits ( ) const;
	unsigned char getMaxDigits ( ) const;
};
#endif
