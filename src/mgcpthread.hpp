#ifndef __MGCPTHREAD_HPP
#define __MGCPTHREAD_HPP
#pragma interface

class EpOriginateThreadHandler;
class CodecInfo;

class MgcpThread : public PThread, public Allocatable < __SS_ALLOCATOR > {
	class Impl;
	Impl * impl;
	PCLASSINFO ( MgcpThread, PThread )
	void Main ( );
public:
	MgcpThread ( const WORD port = 2727 );
	~MgcpThread ( );
	void Close ( );
	void detachThreadHandler ( const ss :: string & gw, const ss :: string & ep, bool ok );
	bool originateCall ( const ss :: string & gw, const ss :: string & ep, EpOriginateThreadHandler * th,
		const ss :: string & callId, const CodecInfo & codec, unsigned telephoneEventsPayloadType, int rtpPort,
		PIPSocket :: Address localAddr );
	void sendRinging ( const ss :: string & gw, const ss :: string & ep, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec );
	void sendOk ( const ss :: string & gw, const ss :: string & ep, int localRtpPort,
		const PIPSocket :: Address & localAddr, const CodecInfo & inCodec, unsigned telephoneEventsPayloadType );
	void reloadState ( );
};
extern MgcpThread * mgcpThread;
#endif
