#ifndef RADIUSCOMMON_HPP_
#define RADIUSCOMMON_HPP_
#pragma interface

class RTPStat;
class CommonCallDetails;
void sendRadiusAcc ( const CommonCallDetails & common, const RTPStat * rtpStats, PTime setupTime, PTime outSetupTime,
	PTime begTime, PTime endTime, int ref, const ss :: string & incomingCallId = ss :: string ( ) );
void externalRoute ( int timeout, const ss :: string & secret, const ss :: string & ani, int inPeer,
	const ss :: string & dialedDigits, IntVector & outPeers, ss :: string & replaceAni );

#endif /*RADIUSCOMMON_HPP_*/
