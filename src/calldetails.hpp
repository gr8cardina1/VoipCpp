#ifndef __CALLDETAILS_HPP
#define __CALLDETAILS_HPP
#pragma interface

struct CallDetails {
	CallDetails ( ) : /*calledPort ( 1720 ), */begInited ( false ), endInited ( false ), begTime ( ),
		endTime ( begTime ), setupInited ( false ), goodCall ( false ), rcSentToOrigin ( false ),
		rcSentToDest ( false ), balancedSeconds ( 0 ), aSent ( false ), pInited ( false ),
		rtpTimeoutStarted ( false ), delayedDisconnect ( false ), fsPort ( 0 ), originatorVersion2 ( false ) { };
	CommonCallDetails common;
	ss :: string convertedDigits;
	unsigned ref;
	H225 :: CallIdentifier id; // guarenteed to be unique, but it may not exist
	bool begInited, endInited;
	PTime begTime, endTime, outSetupTime, setupTime;
	boost :: optional < PTime > pddTime;
	OutTryVector tries;
	Q931 setup, pMesg;
	H225 :: H323_UserInformation uuField, pUUField;
	bool setupInited;
	bool goodCall;
	H225 :: ArrayOf_Asn_OctetString h245Spool, h245SpoolDest;
	bool rcSentToOrigin, rcSentToDest;
	H225 :: ArrayOf_Asn_OctetString fastStart;
	H225 :: ArrayOf_Asn_OctetString fastStartSpool;
	int balancedSeconds;
	bool aSent, pInited;
	EaterDetails eaters;
	bool rtpTimeoutStarted;
	bool delayedDisconnect;
	PTime ddTime;
	OutChoiceDetailsVectorVector choiceForks;
	StringVector forkOutAcctns;
	PIPSocket :: Address fsAddr;
	int fsPort;
	bool originatorVersion2;
};
#endif
