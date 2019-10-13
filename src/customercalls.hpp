#ifndef __CUSTOMERCALLS_HPP
#define __CUSTOMERCALLS_HPP
#pragma interface

class CustomerMoneySpool : public MoneySpool {
	ss :: string uname;
	void destroy ( );
	void propagateBalanceChange ( );
public:
	CustomerMoneySpool ( const ss :: string & u );
};

typedef std :: map < ss :: string, Pointer < CustomerMoneySpool >, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, Pointer < CustomerMoneySpool > > > > StringCustomerMoneySpoolMap;

#endif
