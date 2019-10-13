#ifndef __Q931_HPP
#define __Q931_HPP
#pragma interface

class Q931 {
	class Impl;
	Impl * impl;
public:
	enum MsgTypes {
		msgNationalEscape	= 0x00,
		msgAlerting		= 0x01,
		msgCallProceeding	= 0x02,
		msgProgress		= 0x03,
		msgSetup		= 0x05,
		msgConnect		= 0x07,
		msgSetupAck		= 0x0d,
		msgConnectAck		= 0x0f,
		msgUserInformation	= 0x20,
		msgSuspendReject	= 0x21,
		msgResumeReject		= 0x22,
		msgSuspend		= 0x25,
		msgResume		= 0x26,
		msgSuspendAck		= 0x2d,
		msgResumeAck		= 0x2e,
		msgDisconnect		= 0x45,
		msgRestart		= 0x46,
		msgRelease		= 0x4d,
		msgRestartAck		= 0x4e,
		msgReleaseComplete	= 0x5a,
		msgSegment		= 0x60,
		msgFacility		= 0x62,
		msgNotify		= 0x6e,
		msgStatusEnquiry	= 0x75,
		msgCongestionCtrl	= 0x79,
		msgInformation		= 0x7b,
		msgStatus		= 0x7d
	};
	enum InformationElementCodes {
		ieSegmented		= 0,
		ieBearerCapability	= 0x04,
		ieCause			= 0x08,
		ieFacility		= 0x1c,
		ieProgressIndicator	= 0x1e,
		ieProgressIndicator2	= 0x1f,
		ieCallState		= 0x14,
		ieNotificationIndicator	= 0x27,
		ieDisplay		= 0x28,
		ieKeypad		= 0x2c,
		ieSignal		= 0x34,
		ieConnectedNumber	= 0x4c,
		ieCallingPartyNumber	= 0x6c,
		ieCalledPartyNumber	= 0x70,
		ieRedirectingNumber	= 0x74,
		ieUserUser		= 0x7e,
		ieSendingComplete	= 0xa1
	};
	enum CodingStandard {
		csCCITT,
		csISO,
		csNational,
		csLocal
	};
	enum CauseValues {
		cvUnknown,
		cvUnallocatedNumber			= 1,
		cvNoRouteToNetwork			= 2,
		cvNoRouteToDestination			= 3,
		cvSendSpecialTone			= 4,
		cvMisdialledTrunkPrefix			= 5,
		cvChannelUnacceptable			= 6,
		cvDeliveredInEstablishedChannel		= 7,
		cvNormalCallClearing			= 16,
		cvUserBusy				= 17,
		cvNoResponse				= 18,
		cvNoAnswer				= 19,
		cvSubscriberAbsent			= 20,
		cvCallRejected				= 21,
		cvNumberChanged				= 22,
		cvRedirection				= 23,
		cvExchangeRoutingError			= 25,
		cvNonSelectedUserClearing		= 26,
		cvDestinationOutOfOrder			= 27,
		cvInvalidNumberFormat			= 28,
		cvFacilityRejected			= 29,
		cvStatusEnquiryResponse			= 30,
		cvNormalUnspecified			= 31,
		cvNoCircuitChannelAvailable		= 34,
		cvNetworkOutOfOrder			= 38,
		cvTemporaryFailure			= 41,
		cvCongestion				= 42,
		cvAccesInformationDiscarded		= 43,
		cvRequestedCircitChannelNotAvailable	= 44,
		cvResourceUnavailable			= 47,
		cvBearerCapabilityNotImplemented	= 65,
		cvInvalidCallReference			= 81,
		cvIncompatibleDestination		= 88,
		cvRecoveryOnTimerExpires		= 102,
		cvInterworkingUnspecified		= 111,
		cvNonStandardReason			= 127,
		cvNonStandardReason1			= 149,
		cvError					= 0x100
	};
	enum SignalInfo {
		siDialToneOn,
		siRingBackToneOn,
		siInterceptToneOn,
		siNetworkCongestionToneOn,
		siBusyToneOn,
		siConfirmToneOn,
		siAnswerToneOn,
		siCallWaitingTone,
		siOffhookWarningTone,
		siPreemptionToneOn,
		siTonesOff = 0x3f,
		siAlertingPattern0 = 0x40,
		siAlertingPattern1,
		siAlertingPattern2,
		siAlertingPattern3,
		siAlertingPattern4,
		siAlertingPattern5,
		siAlertingPattern6,
		siAlertingPattern7,
		siAlertingOff = 0x4f,
		siErrorInIE = 0x100
	};
	enum InformationTransferCapability {
		tcSpeech,
		tcUnrestrictedDigital = 8,
		tcRestrictedDigital = 9,
		tc3_1kHzAudio = 16,
		tcUnrestrictedDigitalWithTones = 17,
		tcVideo = 24
	};
	enum UserInfoLayer1 {
		uil1V110 = 1,
		uil1G711m = 2,
		uil1G711A = 3,
		uil1G721 = 4,
		uil1H221 = 5,
		uil1NonCCITT = 7,
		uil1V120 = 8,
		uil1X31 = 9
	};
	enum Location {
		lUser,
		lPrivateLocal,
		lPublicLocal,
		lTransit,
		lPublicRemote,
		lPrivateRemote,
		lInternetworking = 10
	};
	enum ProgressDescription {
		pdNotEndToEndISDN = 1,
		pdDestinationNonISDN,
		pdOriginationNonISDN,
		pdReturnedToISDN,
		pdInternetworking,
		pdInBand = 8
	};
	enum NotificationDescription {
		ndUserSuspended,
		ndUserResumed,
		ndBearerServiceChange
	};

	Q931 ( );

	Q931 ( const Q931 & o );

	Q931 & operator= ( const Q931 & o );

	explicit Q931 ( const ss :: string & data );

	Q931 ( MsgTypes mt, unsigned cr, bool fd, bool dcr = false );

	~Q931 ( );

	void swap ( Q931 & o );

	ss :: string tpkt ( ) const;

	const ss :: string & getIE ( InformationElementCodes ie, std :: size_t n = 0 ) const;
	std :: size_t hasIE ( InformationElementCodes ie ) const;
	void removeIE ( InformationElementCodes ie );
	void setIE ( InformationElementCodes ie, const ss :: string & s );
	void addIE ( InformationElementCodes ie, const ss :: string & s );
	void setCause ( CauseValues cause, unsigned standard = 0, unsigned location = 0 );
	CauseValues getCause ( unsigned * standard = 0, unsigned * location = 0 ) const;
	void setFromDest ( bool v );
	bool getFromDest ( ) const;
	unsigned getCallReference ( ) const;
	bool getDummyCallReference ( ) const;
	void setBearerCapabilities ( InformationTransferCapability capability,
		unsigned transferRate, // Number of 64k B channels
		CodingStandard codingStandard = csCCITT, // 0 = ITU-T standardized coding
		UserInfoLayer1 userInfoLayer1 = uil1H221 // 5 = Recommendations H.221 and H.242
		);
	bool getBearerCapabilities ( InformationTransferCapability & capability, unsigned & transferRate,
		CodingStandard * codingStandard = 0, UserInfoLayer1 * userInfoLayer1 = 0 ) const;
	void setProgressIndicator ( ProgressDescription descr, CodingStandard codingStandard = csCCITT, Location loc = lUser );
	void addProgressIndicator ( ProgressDescription descr, CodingStandard codingStandard = csCCITT,	Location loc = lUser );
	bool getProgressIndicator ( ProgressDescription & descr, std :: size_t n = 0, CodingStandard * codingStandard = 0,
		Location * location = 0 ) const;
	void setSignal ( SignalInfo sig );
	bool getSignal ( SignalInfo & sig ) const;
	void setNotificationDescription ( NotificationDescription nd );
	bool getNotificationDescription ( NotificationDescription & nd ) const;
	ss :: string getCalledPartyNumber ( unsigned * plan = 0, unsigned * type = 0 ) const;
	void setCalledPartyNumber ( const ss :: string & number, unsigned plan = 1, unsigned type = 0 );
	ss :: string getCallingPartyNumber ( unsigned * plan = 0, unsigned * type = 0,
		unsigned * presentation = 0, unsigned * screening = 0, unsigned defPresentation = 0,
		unsigned defScreening = 0 ) const;
	void setCallingPartyNumber ( const ss :: string & number,
		unsigned plan = 1,	// 1 = ISDN/Telephony numbering system
		unsigned type = 0,	// 0 = Unknown number type
		int presentation = - 1,	// 0 = presentation allowed, -1 = no octet3a
		int screening = - 1	// 0 = user provided, not screened
		);
	MsgTypes getMessageType ( ) const;
	void setMessageType ( MsgTypes t );
	ss :: string getMessageTypeName ( ) const;
	static ss :: string getIEName ( InformationElementCodes ie );
	static ss :: string getCauseName ( unsigned cause );
	static ss :: string getUil1Name ( UserInfoLayer1 uil1 );
	static ss :: string getLocationName ( Location loc );
	static ss :: string getProgressDescriptionName ( ProgressDescription descr );
	static ss :: string getCodingStandardName ( CodingStandard cs );
	static ss :: string getInformationTransferCapabilityName ( InformationTransferCapability itc );
	static ss :: string getSignalName ( SignalInfo sig );
	static ss :: string getNotificationDescriptionName ( NotificationDescription nd );
	void printOn ( std :: ostream & os ) const;
};

inline std :: ostream & operator<< ( std :: ostream & os, const Q931 & m ) {
	m.printOn ( os );
	return os;
}

namespace std {
inline void swap ( Q931 & q1, Q931 & q2 ) {
	q1.swap ( q2 );
}
}

#endif
