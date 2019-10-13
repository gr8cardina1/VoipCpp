//TO DO ACCOUNT CALL

/*
LIMITATIONS:
no t120
no RAS
//no fastStart
//no tunneled h245
//messages must have User-User field
//tries only one destination
one call per Q.931 connection ( socket )
no checks for dublicate calls ( table with calldetails and check Ref/Id )
*/

#include <ptlib.h>
#include <ptlib/svcproc.h>
#include <ptlib/sockets.h>
#include "New.h"
//------------------------
#include "ss.hpp"
#include "allocatable.hpp"
#include "Log.h"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "sqloutthread.hpp"
#include "CallThread.h"
#include "ixcudpsocket.hpp"
#include "SipCallThread.h"
#include "confthread.hpp"
#include <csignal>
#include "Version.h"
//#include "regclient.h"
#include "pointer.hpp"
#include "rasthread.hpp"
#include "endthread.hpp"
#include "radiusthread.hpp"
#include "mgcpthread.hpp"
#include "httplistenthread.hpp"
#include "siptransportthread.hpp"
#include <ptlib/pipechan.h>
#include "rtppipe.hpp"
#include <stdexcept>
#include "mysql.hpp"
#include "ssconf.hpp"

RTPPipe * rtpPipe = 0;
Conf * conf = 0;
SQLOutThread * sqlOut = 0;

PCREATE_PROCESS ( opengate )

opengate :: opengate ( ) : PServiceProcess ( "WayaTel", "PSBC",
	GKVER_MAJOR, GKVER_MINOR, GKVER_STATUS, GKVER_BUILD ) { }

BOOL opengate :: OnStart ( ) {
	PSYSTEMLOG ( Info, "OnStart ( )" );
	// Nothing to do...
	return true;
}
// It looks like I need my own signal handler in Unix to stop the PProcess async handler
// killing me before I want to die
void SigHandler ( int sig ) {
	switch( sig ) {
		case SIGINT:
		case SIGTERM:
			if ( Conf :: TerminationSema )
				Conf :: TerminationSema -> Signal ( );
			PSYSTEMLOG ( Error, "Terminate on signal " << sig );
			break;
	}
}

void opengate :: OnControl ( ) {
	// Nothing to do....
}

opengate :: ~opengate ( ) {
//for dumb bsd
	alarm ( 10 );
//	if ( Conf :: TerminationSema )
//		Conf :: TerminationSema -> Wait ( );
}

//copypasted from tlibthrd.cxx
PDECLARE_CLASS(PHouseKeepingThread, PThread)
public:
	PHouseKeepingThread()
		: PThread(1000, NoAutoDeleteThread, NormalPriority, "Housekeeper"), closing ( false )
		{ Resume(); }
	void Main();
	void SetClosing() { closing = TRUE; }
protected:
	BOOL closing;
};
//

void opengate :: Main ( ) {
	std :: signal ( SIGINT, SigHandler );
	std :: signal ( SIGTERM, SigHandler );
	std :: locale :: global ( std :: locale ( "" ) );
	MySQL :: init ( );
	// Set the log level, default is errors only (1) for normal builds and info(3) for
	// debug builds
#ifdef _DEBUG
	SetLogLevel ( PSystemLog :: Info );
	PTrace :: SetLevel ( 3 );
#else
	SetLogLevel ( PSystemLog :: Error );
	PTrace :: SetLevel ( 2 );
#endif

	ConfThread * ct = 0;
	EndThread * et = 0;
	try {
//		PSYSTEMLOG ( Info, "before client_verify" );
//#ifndef linux
//		if ( client_verify ( 1 ) )
//			throw std :: runtime_error ( "unknown exception" );
//#endif
		rtpPipe = new RTPPipe;
		conf = new Conf;
	} catch ( std :: exception & e ) {
		PSYSTEMLOG ( Error, "exception: " << e.what ( ) );
		return;
	} catch ( ... ) {
		PSYSTEMLOG ( Error, "unknown exception" );
		return;
	}
	Log = new OpengateLog;
	ct = new ConfThread ( );
	et = new EndThread ( );
	radiusLog = new OpengateLog ( "/opt/psbc/logs/psbc_rad.log" );
	new SQLOutThread ( sqlOut );
	// Create the call signalling thread
	CallThread * CThread = new CallThread( );
//	SipCallThread * SipCThread = new SipCallThread( );
	new SIP2 :: TransportThread ( );
	conf -> startGateKeepers ( );
	RasThread * ras = new RasThread ( );
	RadiusThread * rad = 0;
	if ( startRadius )
		rad = new RadiusThread ( );
	MgcpThread * mgcp = 0;
	if ( ! noMgcp )
		mgcp = new MgcpThread ( );
	HTTPListenThread * http = new HTTPListenThread ( );

	// Wait for death....
	if ( Conf :: TerminationSema )
		Conf :: TerminationSema -> Wait ( );
	conf -> setShuttingDown ( );
	// Clean up
	CThread -> Close ( );
//	SipCThread -> Close ( );
	SIP2 :: transportThread -> Close ( ); // nado bi predvaritelno zvonki pozakrivat i transactions zakonchit ?
	ras -> Close ( );
	if ( rad )
		rad -> Close ( );
	if ( mgcp )
		mgcp -> Close ( );
	http -> Close ( );
	conf -> waitForSignallingChannels ( );
	sqlOut -> Close ( );
	housekeepingThread -> SetClosing ( );
	SignalTimerChange ( );
	CThread -> WaitForTermination ( );
//	SipCThread -> WaitForTermination ( );
	SIP2 :: transportThread -> WaitForTermination ( );
	ras -> WaitForTermination ( );
	if ( rad )
		rad -> WaitForTermination ( );
	if ( mgcp )
		mgcp -> WaitForTermination ( );
	http -> WaitForTermination ( );
	Sleep ( 3000 );
	delete CThread;
//	delete SipCThread;
	safeDel ( SIP2 :: transportThread );
	delete ras;
	delete rad;
	delete mgcp;
	delete http;
	sqlOut -> WaitForTermination ( );
	Sleep ( 3000 );
	delete sqlOut;
	ct -> WaitForTermination ( );
	delete ct;
	delete rtpPipe;
	et -> WaitForTermination ( );
	delete et;
	housekeepingThread -> WaitForTermination ( );
	safeDel ( housekeepingThread );
	// Wait for death
	std :: signal ( SIGINT, SIG_DFL );
	std :: signal ( SIGTERM, SIG_DFL );
	safeDel ( Conf :: TerminationSema );
	safeDel ( Conf :: TerminatedSema );
	delete conf;
	delete Log;
	delete radiusLog;
	PSYSTEMLOG ( Info, "PSBC ended" );
	MySQL :: end ( );
	Sleep ( 1000 );
//	CommonDestruct ( );
	std :: locale :: global ( std :: locale :: classic ( ) );
}
