#ifndef __MGCP_HPP
#define __MGCP_HPP
#pragma interface

namespace SDP {
	class SessionDescription;
}

namespace MGCP {

typedef std :: map < ss :: string, StringVector, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, StringVector > > > StringStringVectorMap;

class Capabilities {
public:
	Capabilities ( const ss :: string & s );
	void addHeader ( const ss :: string & n, const ss :: string & v );
	void printOn ( std :: ostream & os ) const;
	void readFrom ( std :: istream & is );
	void clear ( );
	bool getHeader ( const ss :: string & n, StringVector & s ) const;
	bool getHeader ( const ss :: string & n, ss :: string & s ) const;
	Capabilities & operator= ( const ss :: string & s );
	ss :: string str ( ) const;
private:
	StringStringVectorMap headers;
};

inline std :: ostream & operator<< ( std :: ostream & os, const Capabilities & c ) {
	c.printOn ( os );
	return os;
}

inline std :: istream & operator>> ( std :: istream & is, Capabilities & c ) {
	c.readFrom ( is );
	return is;
}

class MIMEInfo {
public:
	void swap ( MIMEInfo & m );
	void addHeader ( const ss :: string & n, const ss :: string & v );
	void removeHeader ( const ss :: string & n );
	void printOn ( std :: ostream & os ) const;
	void readFrom ( std :: istream & is );
	void clear ( );
	bool getHeader ( const ss :: string & n, StringVector & s ) const;
	bool getHeader ( const ss :: string & n, ss :: string & s ) const;
	bool hasHeader ( const ss :: string & n ) const;
	bool getSpecificEndPointId ( StringVector & s ) const;
	bool getSpecificEndPointId ( ss :: string & s ) const;
	bool getConnectionId ( ss :: string & s ) const;
	bool getSecondConnectionId ( ss :: string & s ) const;
	bool getRestartMethod ( ss :: string & s ) const;
	bool getRestartDelay ( int & d ) const;
	bool getRequestIdentifier ( ss :: string & s ) const;
	bool getRequestedEvents ( ss :: string & s ) const;
	bool getCapabilities ( Capabilities & c ) const;
	bool getCapabilities ( std :: vector < Capabilities, __SS_ALLOCATOR < Capabilities > > & c ) const;
	bool getObservedEvents ( ss :: string & s ) const;
	bool getResponseAck ( ss :: string & s ) const;
	bool getEventStates ( ss :: string & s ) const;
	bool hasConnectionId ( ) const;
	bool hasResponseAck ( ) const;
	void setMode ( const ss :: string & v );
	void setCallId ( const ss :: string & v );
	void setSecondEndPointId ( const ss :: string & v );
	void setRequestIdentifier ( const ss :: string & v );
	void setRequestedEvents ( const ss :: string & v );
	void setCapabilities ( const Capabilities & v );
	void setRequestedInfo ( const ss :: string & v );
	void setPersistentEvents ( const ss :: string & v );
	void setConnectionId ( const ss :: string & s );
	void setResponseAck ( const ss :: string & s );
	void setSignalRequests ( const ss :: string & s );
	void setDigitMap ( const ss :: string & s );
	void setNotifiedEntity ( const ss :: string & n );
	void setLocalConnectionOptions ( const ss :: string & n );
	void removeRequestedEvents ( );
	void removeRequestedInfo ( );
private:
	StringStringVectorMap headers;
};

inline std :: ostream & operator<< ( std :: ostream & os, const MIMEInfo & m ) {
	m.printOn ( os );
	return os;
}

inline std :: istream & operator>> ( std :: istream & is, MIMEInfo & m ) {
	m.readFrom ( is );
	return is;
}

inline void swap ( MIMEInfo & m1, MIMEInfo & m2 ) {
	m1.swap ( m2 );
}

class PDU {
public:
	enum Verbs {
		vEndpointConfiguration,
		vCreateConnection,
		vModifyConnection,
		vDeleteConnection,
		vNotificationRequest,
		vNotify,
		vAuditEndpoint,
		vAuditConnection,
		vRestartInProgress,
		vMessage,
		vNum
	};
	enum ReturnCodes {
		rcInvalid = 1000,
		rcResponseAck = 0,
		rcProvisional = 100,
		rcOK = 200,
		rcDeleted = 250,
		rcOffHook = 401,
		rcOnHook = 402,
		rcUnknownEndpoint = 500,
		rcNoResources = 502,
		rcAllOfTooComplicated = 503,
		rcUnknownCommand = 504,
		rcProtocolError = 510,
		rcCantDetect = 512,
		rcNoSuchEvent = 522,
		rcNoRemoteConnection = 527,
		rcIncompatibleProtocolVersion = 528,
		rcUnknownRestartMethod = 536
	};
	PDU ( const ss :: string & e, const ss :: string & g, const ss :: string & i, unsigned short p, Verbs v );
	PDU ( ReturnCodes rc, unsigned i, const ss :: string & a, unsigned short p );
	PDU ( const PDU & p );
	explicit PDU ( const ss :: string & i = ss :: string ( ), unsigned short p = 0 );
	PDU & operator= ( const PDU & p );
	void swap ( PDU & p );
	ss :: string str ( ) const;
	void printOn ( std :: ostream & os ) const;
	void readFrom ( std :: istream & is );
	~PDU ( );
	const MIMEInfo & getMIME ( ) const;
	MIMEInfo & getMIME ( );
	void setMIME ( const MIMEInfo & m );
	void setEndpoint ( const ss :: string & s );
	void newId ( );
	void setVerb ( Verbs v );
	void setCode ( ReturnCodes c, const ss :: string & com = "" );
	void setSDP ( SDP :: SessionDescription * s );
	void setSDP ( const SDP :: SessionDescription & s );
	unsigned getId ( ) const;
	ReturnCodes getCode ( ) const;
	Verbs getVerb ( ) const;
	ss :: string getEndpointGw ( ) const;
	const ss :: string & getIp ( ) const;
	unsigned short getPort ( ) const {
		return port;
	}
	const SDP :: SessionDescription * getSDP ( ) const;
	void setGw ( const ss :: string & g ) {
		gw = g;
	}
	const ss :: string & getGw ( ) const {
		return gw;
	}
	const ss :: string & getEp ( ) const {
		return ep;
	}
	const ss :: string & getVerbName ( ) const;
	const ss :: string & getComment ( ) const;
private:
	unsigned id;
	static unsigned curTrans;
	static unsigned getNewId ( );
	static const char * getComment ( ReturnCodes rc );
	ss :: string ep;
	ss :: string gw;
	ss :: string ip;
	unsigned short port;
	Verbs verb;
	ReturnCodes rc;
	ss :: string comment;
	MIMEInfo mime;
	SDP :: SessionDescription * sdp;
};

typedef std :: vector < MGCP :: PDU, __SS_ALLOCATOR < MGCP :: PDU > > PduVector;

inline std :: ostream & operator<< ( std :: ostream & os, const PDU & p ) {
	p.printOn ( os );
	return os;
}

inline std :: istream & operator>> ( std :: istream & is, PDU & p ) {
	p.readFrom ( is );
	return is;
}

inline void swap ( PDU & p1, PDU & p2 ) {
	p1.swap ( p2 );
}

}

void makeResponse ( MGCP :: PduVector & pdus, const MGCP :: PDU & pdu, MGCP :: PDU :: ReturnCodes rc );

#endif
