#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptclib/http.h>
#include <ptclib/httpform.h>
#include "totalactiveresource.hpp"
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>


TotalActiveResource :: TotalActiveResource ( ) :
	PHTTPResource ( "total.xml", "text/xml" ) { }

PString TotalActiveResource :: LoadText ( PHTTPRequest & request) {


	ss :: ostringstream os;
	conf -> printActiveCallsStats ( os );
	PSYSTEMLOG(Info, "TotalActiveResource: request: " << request.entityBody << "; response" << os.str ( ).c_str ( ) );
	return os.str ( ).c_str ( );
}

BOOL TotalActiveResource :: LoadHeaders ( PHTTPRequest & /*request*/ ) {
	return true;
}
