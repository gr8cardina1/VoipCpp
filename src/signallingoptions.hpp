#ifndef __SIGNALLINGOPTIONS_HPP
#define __SIGNALLINGOPTIONS_HPP
#pragma interface

enum TunnelingWithH245Address {
	useBoth,
	useTunneling,
	useH245Address
};

class SigOptionsPeer {
	friend class SigOptionsCall;
public:
		// enum for supportVia values
		enum ViaSupport
		{
			vsEmpty = 2,	//not initialized yet
			vsNo = 0,		//do not suport
			vsYes = 1
		};

//	~SigOptionsPeer();
	SigOptionsPeer ( ) : dropTunneling ( false ), tunnelingWithH245Address ( useBoth ),
		dropProgressBeforeAlerting ( false ), dropNonStandard ( true ), dropFastStart ( false ),
		rtpTimeoutEnable ( true ), answerToRTDR ( false ), permitTunnelingInSetup ( false ), rtpTimeoutBoth ( 60 ),
		rtpTimeoutOne ( 180 ), rtpTimeoutPeriod ( 30 ), connectTimeout ( 180 ), maxCallDurationMin ( 0 ),
		delayedDisconnect ( false ), addBearerToConnect ( false ),
		emptyFacility ( false ), removeSymmetricOperation ( false ),
		replaceCallProceedingWithAlerting ( false ), useNormalizer ( false ), supportVia(vsEmpty),
		isPAssertIdRequired_(false), isTrusted_(false), isRecordRouteRequired_(false),
		isUseCNonce_(false), isForceInsertingDTMFintoInvite_( true ), addDestinationAddress ( true ), isAccSend_( false ),
		useRemotePartyID ( false ), aceptedDTMF_( true ) { }
	SigOptionsPeer ( const char * * p ) : dropTunneling ( p [ 0 ] [ 0 ] == 'y' ),
		tunnelingWithH245Address ( p [ 1 ] [ 3 ] == 'B' ? useBoth :
		( p [ 1 ] [ 3 ] == 'T' ? useTunneling : useH245Address ) ),
		dropProgressBeforeAlerting ( p [ 2 ] [ 0 ] == 'y' ),
		dropNonStandard ( p [ 3 ] [ 0 ] == 'y' ), dropFastStart ( p [ 4 ] [ 0 ] == 'y' ),
		rtpTimeoutEnable ( p [ 5 ] [ 0 ] == 'y' ), answerToRTDR ( p [ 12 ] [ 0 ] == 'y' ),
		permitTunnelingInSetup ( p [ 15 ] [ 0 ] == 'y' ), rtpTimeoutBoth ( atoi ( p [ 6 ] ) ),
		rtpTimeoutOne ( atoi ( p [ 7 ] ) ), rtpTimeoutPeriod ( atoi ( p [ 8 ] ) ),
		connectTimeout ( atoi ( p [ 13 ] ) ), maxCallDurationMin ( atoi ( p [ 14 ] ) ),
		delayedDisconnect ( p [ 11 ] [ 0 ] == '_' ), addBearerToConnect ( p [ 16 ] [ 0 ] == 'y' ),
		emptyFacility ( p [ 17 ] [ 0 ] == 'y' ), removeSymmetricOperation ( p [ 18 ] [ 0 ] == 'y' ),
		replaceCallProceedingWithAlerting ( p [ 19 ] [ 0 ] == 'y' ), useNormalizer ( p [ 20 ] [ 0 ] == 'y' ),
		supportVia ( p [ 21 ] [ 0 ] == 'y' ? vsYes : vsNo ),
		isPAssertIdRequired_(p [ 22 ] [ 0 ] == 'y'), isTrusted_(p [ 23 ] [ 0 ] == 'y'),
		isRecordRouteRequired_ ( p [ 24 ] [ 0 ] == 'y' ), isUseCNonce_ ( false ),
		isForceInsertingDTMFintoInvite_ ( p [ 25 ] [ 0 ] == 'y' ), addDestinationAddress ( p [ 26 ] [ 0 ] == 'y' ),
		startH245 ( p [ 27 ] [ 0 ] == 'y' ), useFacility ( p [ 28 ] [ 0 ] == 'y' ), isAccSend_( p [ 29 ][ 0 ] == 'y' ),
		useRemotePartyID ( p [ 30 ] [ 0 ] == 'y' ), aceptedDTMF_ ( p [ 31 ] [ 0 ] == 'y' )
		 { }
	bool getDropTunneling ( ) const {
		return dropTunneling;
	}
	bool getDropFastStart ( ) const {
		return dropFastStart;
	}
	bool getPermitTunnelingInSetup ( ) const {
		return permitTunnelingInSetup;
	}
	bool getReplaceCallProceedingWithAlerting ( ) const {
		return replaceCallProceedingWithAlerting;
	}
	bool getUseNormalizer ( ) const {
		return useNormalizer;
	}
	ViaSupport getSupportVia(void) const
	{
		return supportVia;
	}
	bool isPAssertIdRequired() const
	{
		return isPAssertIdRequired_;
	}
	bool isTrusted() const
	{
		return isTrusted_;
	}
	bool isRecordRouteRequired ( ) const {
		return isRecordRouteRequired_;
	}

	bool isUseCNonce() const
	{
		return isUseCNonce_;
	}

	bool isForceInsertingDTMFintoInvite () const
	{
		return isForceInsertingDTMFintoInvite_;
	}
	bool getAddDestinationAddress ( ) const {
		return addDestinationAddress;
	}
	bool getStartH245 ( ) const {
		return startH245;
	}
	bool getUseFacility ( ) const {
		return useFacility;
	}

	bool isAccSend ( ) const {
		return isAccSend_;
	}
	bool getUseRemotePartyID ( ) const {
		return useRemotePartyID;
	}
	
	bool isAceptedDTMF() const {
		return aceptedDTMF_;
	}	
	
//	SigOptionsPeer ( const SigOptionsPeer& );
//	const SigOptionsPeer& operator= ( const SigOptionsPeer& );
//
//	static int __CountOfSigOptionsPeer__;
//	static int __CurrentSigOptionsPeer__;

private:

//	int __numberThisInstance__;

	bool dropTunneling;
	TunnelingWithH245Address tunnelingWithH245Address;
	bool dropProgressBeforeAlerting;
	bool dropNonStandard;
	bool dropFastStart;
	bool rtpTimeoutEnable;
	bool answerToRTDR;
	bool permitTunnelingInSetup;
	int rtpTimeoutBoth;
	int rtpTimeoutOne;
	int rtpTimeoutPeriod;
	int connectTimeout;
	int maxCallDurationMin;
	bool delayedDisconnect;
	bool addBearerToConnect;
	bool emptyFacility;
	bool removeSymmetricOperation;
	bool replaceCallProceedingWithAlerting;
	bool useNormalizer;
	ViaSupport supportVia;
	bool isPAssertIdRequired_;
	bool isTrusted_;
	bool isRecordRouteRequired_;
	bool isUseCNonce_;
	bool isForceInsertingDTMFintoInvite_;
	bool addDestinationAddress;
	bool startH245;
	bool useFacility;
	bool isAccSend_;
	bool useRemotePartyID;
	bool aceptedDTMF_;
};

class SigOptionsCall {
	SigOptionsPeer in, out;
public:
	void setIn ( const SigOptionsPeer & o ) {
		in = o;
	}
	void setOut ( const SigOptionsPeer & o ) {
		out = o;
	}
	bool dropTunneling ( ) const {
		return in.dropTunneling || out.dropTunneling;
	}
	TunnelingWithH245Address tunnelingWithH245Address ( ) const {
		if ( in.tunnelingWithH245Address == useBoth )
			return out.tunnelingWithH245Address;
		return in.tunnelingWithH245Address;
	}
	bool dropProgressBeforeAlerting ( ) const {
		return in.dropProgressBeforeAlerting || out.dropProgressBeforeAlerting;
	}
	bool dropNonStandard ( ) const {
		return in.dropNonStandard || out.dropNonStandard;
	}
	bool dropFastStart ( ) const {
		return in.dropFastStart || out.dropFastStart;
	}
	bool rtpTimeoutEnable ( ) const {
		return in.rtpTimeoutEnable || out.rtpTimeoutEnable;
	}
	int rtpTimeoutBoth ( ) const {
		return std :: min ( in.rtpTimeoutBoth, out.rtpTimeoutBoth );
	}
	int rtpTimeoutOne ( ) const {
		return std :: min ( in.rtpTimeoutOne, out.rtpTimeoutOne );
	}
	int rtpTimeoutPeriod ( ) const {
		return std :: max ( std :: min ( in.rtpTimeoutPeriod, out.rtpTimeoutPeriod ), 3 );
	}
	bool inAnswerToRTDR ( ) const {
		return in.answerToRTDR;
	}
	bool outAnswerToRTDR ( ) const {
		return out.answerToRTDR;
	}
	int connectTimeout ( ) const {
		return std :: min ( in.connectTimeout, out.connectTimeout );
	}
	int maxCallDurationMin ( ) const {
		if ( ! in.maxCallDurationMin )
			return out.maxCallDurationMin;
		if ( ! out.maxCallDurationMin )
			return in.maxCallDurationMin;
		return std :: min ( in.maxCallDurationMin, out.maxCallDurationMin );
	}
	bool permitTunnelingInSetup ( ) const {
		return in.permitTunnelingInSetup || out.permitTunnelingInSetup;
	}
	bool delayedDisconnect ( ) const {
		return out.delayedDisconnect || in.delayedDisconnect;
	}
	bool addBearerToConnect ( ) const {
		return out.addBearerToConnect || in.addBearerToConnect;
	}
	bool emptyFacility ( ) const {
		return out.emptyFacility || in.emptyFacility;
	}
	bool removeSymmetricOperation ( ) const {
		return out.removeSymmetricOperation || in.removeSymmetricOperation;
	}
	bool replaceCallProceedingWithAlerting ( ) const {
		return out.replaceCallProceedingWithAlerting;
	}
	bool getUseNormalizer ( ) const {
		return in.useNormalizer;
	}
	bool getSupportVia ( ) const {
		return in.getSupportVia ( );
	}
	bool isPAssertIdRequired ( ) const {
		return in.isPAssertIdRequired ( );
	}
	bool isTrusted ( ) const {
		return out.isTrusted ( );
	}
	bool isRecordRouteRequired ( ) const {
		return in.isRecordRouteRequired ( ) || out.isRecordRouteRequired ( );
	}
	bool isRecordRouteRequiredOrig ( ) const {
		return in.isRecordRouteRequired ( );
	}
	bool isRecordRouteRequiredTerm ( ) const {
		return out.isRecordRouteRequired ( );
	}

	bool isForceInsertingDTMFintoInviteOrig () const {
		return in.isForceInsertingDTMFintoInvite();
	}
	bool isForceInsertingDTMFintoInviteTerm () const {
		return out.isForceInsertingDTMFintoInvite();
	}
	bool isForceInsertingDTMFintoInvite ( ) const {
		return isForceInsertingDTMFintoInviteOrig ( ) || isForceInsertingDTMFintoInviteTerm ( );
	}
	bool addDestinationAddress ( ) const {
		return out.getAddDestinationAddress ( );
	}
	bool startH245 ( ) const {
		return in.getStartH245 ( );
	}
	bool useFacility ( ) const {
		return in.getUseFacility ( );
	}
	bool isAccSend ( ) const {
		return out.isAccSend ( );
	}
	bool getUseRemotePartyID ( ) const {
		return in.getUseRemotePartyID ( ) || out.getUseRemotePartyID ( );
	}
	bool getAceptedDTMF() const {
		return out.isAceptedDTMF();
	}	
	
};

#endif
