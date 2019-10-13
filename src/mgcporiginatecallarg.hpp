#ifndef MGCPORIGINATECALLARG_HPP_
#define MGCPORIGINATECALLARG_HPP_
#pragma interface
struct OriginateCallArg : boost :: noncopyable {
	MGCP :: PduVector & ret;
	EpOriginateThreadHandler * th;
	const ss :: string & callId;
	const CodecInfo & codec;
	unsigned telephoneEventsPayloadType;
	int rtpPort;
	const ss :: string & mediaIp;
	const ss :: string & signalIp;
	OriginateCallArg ( MGCP :: PduVector & r, EpOriginateThreadHandler * t, const ss :: string & ci, const CodecInfo & co,
		unsigned pt, int rp, const ss :: string & mi, const ss :: string & si ) : ret ( r ), th ( t ), callId ( ci ),
		codec ( co ), telephoneEventsPayloadType ( pt ), rtpPort ( rp ), mediaIp ( mi ), signalIp ( si ) { }
};

#endif /*MGCPORIGINATECALLARG_HPP_*/
