#ifndef __SIGNALLINGTHREAD_HPP
#define __SIGNALLINGTHREAD_HPP
#pragma interface

class H245Thread;
class H245Handler;
class SignallingThread : public PThread, public CallControl, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( SignallingThread, PThread )
public:
	SignallingThread ( PTCPSocket * recvSocket, unsigned _id );
	virtual ~SignallingThread ( );
protected:
	int getCallSeconds ( ) const;
	void beginShutDown ( );
	bool getCallConnected ( ) const;
	int getCallRef ( ) const;
	ss :: string getInIp ( ) const;
	int getInPeerId ( ) const;
	ss :: string getInPeerName ( ) const;
	ss :: string getInResponsibleName ( ) const;
	ss :: string getSetupTime ( ) const;
	ss :: string getConnectTime ( ) const;
	ss :: string getOutIp ( ) const;
	int getOutPeerId ( ) const;
	ss :: string getCallId ( ) const;
	ss :: string getOutPeerName ( ) const;
	ss :: string getOutResponsibleName ( ) const;
	ss :: string getCallingDigits ( ) const;
	ss :: string getSentDigits ( ) const;
	ss :: string getDialedDigits ( ) const;
	ss :: string getInCode ( ) const;
	ss :: string getOutCode ( ) const;
	ss :: string getInAcctn ( ) const;
	ss :: string getOutAcctn ( ) const;
	ss :: string getPdd ( ) const;
	bool getDifferentProtocol ( ) const;
	bool getDifferentCodec ( ) const;
	SignallingThread ( ); // Stop clients calling default cons
	SignallingThread ( const SignallingThread & ); // ...or copy constructor

	virtual void Main ( );

	void Close ( );

	bool receiveMesg ( bool fromCaller );
	// Task: to receive a Q931 message

	bool handleMesg ( Q931 & mesg, bool fromCaller );
	// Task: to handle the given Q931 message

	bool handleH245Setup ( H225 :: H323_UserInformation & uuField, bool fromCaller );
	// Task: to handle potential setting up of H.245 channel

	// Override the following functions if you want to do fancy
	// things...

	bool handleSetup ( const Q931 & mesg, const H225 :: H323_UserInformation & uuField );
	// Task: to handle a setup message
	// Retn: true iff message should be forwarded

	bool handleProceeding ( Q931 & mesg, H225 :: H323_UserInformation & uuField );
	// Task: to handle a call proceeding
	// Retn: true iff message should be forwarded

 	bool handleAlerting ( Q931 & mesg, H225 :: H323_UserInformation & uuField );
	// Task: to handle an alerting message
	// Retn: true iff message should be forwarded

	bool handleConnect ( Q931 & mesg, H225 :: H323_UserInformation & uuField );
	// Task: to handle a connect message
	// Retn: true iff message should be forwarded

	bool handleRelease ( Q931 & mesg, const H225 :: H323_UserInformation * uuField, bool fromCaller,
		bool & end );
	// Task: to handle a release complete message
	// Retn: true iff message should be forwarded

	bool handleFacility ( Q931 & mesg, H225 :: H323_UserInformation & uuField, bool fromCaller );
	// Task: to handle a facility message
	// Retn: true iff message should be forwarded

	bool handleProgress ( Q931 & mesg, H225 :: H323_UserInformation & uuField, bool fromCaller );
	bool handleProgressReal ( Q931 & mesg, H225 :: H323_UserInformation & uuField, bool fromCaller );
	// Task: to handle a progress message
	// Retn: true iff message should be forwarded

	//void ConnectToDestination( const Q931 & SetupMesg );
//	bool ConnectToDestination( );
	// Task: to connect the socket to the desired destination of the
	// given setup message

	bool getCallDetailsForSetup ( );
	// Task: to obtain a call details and destination endpoint relative
	// to the SetupMesg. Search the CallDetails object in the calls
	// table and if not found try to create a new call item.

	void getChoices ( );
	bool tryNextChoice ( );
	bool tryNextChoiceIteration ( );
	void sendReleaseComplete ( PTCPSocket * sock = 0 );
	void getSetup ( );
	void sendSetup ( );
	void closeThisChoice ( );
	void sendProceeding ( );
	void sqlStart ( );
	void sqlEnd ( );
	void shutDown ( );
	void handleH245 ( H225 :: H323_UU_PDU & pdu, bool goodCall, bool fromCaller );
	void handleH245 ( H225 :: ArrayOf_Asn_OctetString & control, bool fromCaller );
	void translateSetup ( Q931 & mesg, H225 :: H323_UserInformation & uuField,
		const H225 :: TransportAddress & sourceCallSigAddr,
		const H225 :: TransportAddress & destCallSigAddr );
	template < class T > void handleFastStartResponse ( T & uuie );
	H225 :: ArrayOf_Asn_OctetString handleFastStart ( );
	H245Handler * createH245Handler ( );
	bool directRTP ( ) const;
	void accountCall ( );
	void sendMesgToDest ( const Q931 & Mesg, const H225 :: H323_UserInformation * uuField,
		PTCPSocket * Destination );
	template < class T > void handleCallId ( T & uuie );
	void getInPrice ( const H225 :: H323_UU_PDU & pdu );
	void sendTunneling ( );
	void sendKeepalive ( );
	void startH245 ( H225 :: H323_UserInformation & uuField );
	void handleProgressIndicator ( const Q931 & mesg );
	void calledSocketConnected ( );
	unsigned id;
	PTCPSocket * callerSocket;
	PTCPSocket * calledSocket;
	H245Thread * myH245Thread; // To handle routed H.245
	H245Handler * h;
	friend class Conf;
	CallDetails call;
	CommonCallDetails & common;
	bool directOut;
	RTPStat rtpStats [ 4 ];
	StringVector progress;
	bool sourceIsVoIP;
	bool setupTimeSaved;
	FakeCallDetector _fakeCallDetector;
	bool _RTDRsendBack;
	volatile bool differentCodec;
};
#endif
