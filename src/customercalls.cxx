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
#include "customercalls.hpp"
#include <ptlib/sockets.h>
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"

void CustomerMoneySpool :: destroy ( ) {
	conf -> destroyCustomerSpool ( uname );
}

void CustomerMoneySpool :: propagateBalanceChange ( ) {
	conf -> propagateCustomerBalanceChange ( uname, getTotalMoney ( ) );
}

CustomerMoneySpool :: CustomerMoneySpool ( const ss :: string & u ) : uname ( u ) { }

