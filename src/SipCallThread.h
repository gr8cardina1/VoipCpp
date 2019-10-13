#ifndef _SIPCALLTHREAD_H
#define _SIPCALLTHREAD_H
#pragma interface

// This thread handles the dispatching of connections to new signalling threads
class SipCallThread : public PThread, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( SipCallThread, PThread )
public:
	SipCallThread ( );

	virtual ~SipCallThread ( );

	void Close ( );

protected:

	virtual void Main ( );

	IxcUDPSocket CallSocket;
};

#endif // _SIPCALLTHREAD_H
