#if !defined(AFX_PRASLOG_H__0BE851A7_613E_4662_A0F8_3C460C279896__INCLUDED_)
#define AFX_PRASLOG_H__0BE851A7_613E_4662_A0F8_3C460C279896__INCLUDED_
#pragma interface

namespace H225 {
	class RasMessage;
	class TransportAddress;
	class H323_UserInformation;
}
namespace H245 {
	class MultimediaSystemControlMessage;
}

class Q931;

namespace SIP {
	class PDU;
}

namespace SIP2 {
	class Response;
	class Request;
}

namespace MGCP {
	class PDU;
}

namespace Radius {
	class Request;
}


class OpengateLog : public Allocatable < __SS_ALLOCATOR > {
	public:
	enum Direction {
		Sending,
		Receiving
	};
	void logNote ( const ss :: string & note);
	void logRasMsg ( const H225 :: RasMessage & msg, Direction Dir, const H225 :: TransportAddress & addr );
	void logQ931Msg ( const Q931 & msg, const H225::H323_UserInformation * uuField, Direction Dir,
		const PIPSocket :: Address & address, WORD port, bool forceDebug = false );
	void LogSIPMsg ( SIP :: PDU & msg, Direction Dir, const PIPSocket :: Address & address,
		WORD port, bool forceDebug = false );
	void LogSIP2Msg ( const SIP2 :: Request & msg, Direction Dir, const PIPSocket :: Address & address,
		WORD port, bool forceDebug = false );
	void LogSIP2Msg ( const SIP2 :: Response & msg, Direction Dir, const PIPSocket :: Address & address,
		WORD port, bool forceDebug = false );
	void logMGCPMsg ( const MGCP :: PDU & msg, Direction Dir, const PIPSocket :: Address & address,
		WORD port, bool forceDebug = false );
	void logMGCPMsg ( const MGCP :: PDU & msg, Direction Dir, const ss :: string & address,
		WORD port, bool forceDebug = false );
	void logH245Msg ( const H245 :: MultimediaSystemControlMessage & msg, Direction Dir,
		const PIPSocket :: Address & address, WORD port, bool forceDebug = false );
	void logRadiusMsg ( const Radius :: Request & msg, Direction Dir,
		bool forceDebug = false );
/// define the different trace log levels
	enum Level {
		//No logging
		None,
		/// Log message summary only
		Info,
		/// Log message details
		Trace,
		NumLogLevels
	};
	OpengateLog ( const ss :: string & nm = "/opt/psbc/logs/psbc_proto.log" );
	virtual ~OpengateLog ( );
protected:
	int getLogLevel ( ) const;
	// Task: to return the current logging level
	void writeHeader ( );
	// Task: to write the header information for a log message
	void writeHeader ( Direction Dir );
	// Task: as above including direction....
	void writeHeader ( Direction Dir, const PIPSocket :: Address & address, WORD port );
	void writeHeader ( Direction Dir, const ss :: string & address, WORD port );
	// Task: ...and address of sender / receiver
	void writeHeader ( Direction Dir, const H225 :: TransportAddress & addr );
	void writeTrailer ( );
	// Task: to write the trailer for a log message
	template < class T > void logFastStart ( const T & uuie );

	PFilePath logFilename;
	PTextFile logFile;
	PMutex logMutex;
	void open ( );
	void close ( );
	void logQ931MsgInt ( const Q931 & msg, const H225 :: H323_UserInformation * uuField, bool forceDebug );
	void LogSIPMsgInt ( SIP :: PDU & msg, bool forceDebug );
	void LogSIP2MsgInt ( const SIP2 :: Request & msg, bool forceDebug );
	void LogSIP2MsgInt ( const SIP2 :: Response & msg, bool forceDebug );
	void logMGCPMsgInt ( const MGCP :: PDU & msg, bool forceDebug );
	void logBearerCapability ( const Q931 & msg );
	void logProgressIndicator ( const Q931 & msg );
	void logSignal ( const Q931 & msg );
	void logNotificationIndicator ( const Q931 & msg );
};
extern OpengateLog * Log, * radiusLog;

#endif // !defined(AFX_PRASLOG_H__0BE851A7_613E_4662_A0F8_3C460C279896__INCLUDED_)
