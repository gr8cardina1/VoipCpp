#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptclib/http.h>
#include "activeresource.hpp"
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>


ActiveResource :: ActiveResource ( ) :
	PHTTPResource ( "index.xml", "text/xml" ) { }

PString ActiveResource :: LoadText ( PHTTPRequest & /*request*/ ) {
	ss :: ostringstream os;
	conf -> printActiveCalls ( os );
	return os.str ( ).c_str ( );
}

BOOL ActiveResource :: LoadHeaders ( PHTTPRequest & /*request*/ ) {
	return true;
}

ActiveResourceCSV :: ActiveResourceCSV ( ) :
	PHTTPResource ( "index.csv", "text/csv" ) { }

PString ActiveResourceCSV :: LoadText ( PHTTPRequest & /*request*/ ) {
	ss :: ostringstream os;
	conf -> printActiveCallsCSV ( os );
	return os.str ( ).c_str ( );
}

BOOL ActiveResourceCSV :: LoadHeaders ( PHTTPRequest & /*request*/ ) {
	return true;
}

