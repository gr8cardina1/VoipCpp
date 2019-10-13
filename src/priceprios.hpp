#ifndef PRICEPRIOS_HPP_
#define PRICEPRIOS_HPP_
#pragma interface

class OutChoice;
struct PricePrios {
	int ( OutChoice ::* f1 ) ( ) const;
	int ( OutChoice ::* f2 ) ( ) const;
	int ( OutChoice ::* f3 ) ( ) const;
	PricePrios ( );
};
#endif /*PRICEPRIOS_HPP_*/
