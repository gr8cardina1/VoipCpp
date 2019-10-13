#ifndef __LEGTHREAD_HPP
#define __LEGTHREAD_HPP
#pragma interface

class H323Call;
class LegThread : public PThread, public Allocatable < __SS_ALLOCATOR >, boost :: noncopyable {
	PCLASSINFO ( LegThread, PThread )
protected:
	H323Call * call;
	PTime begTime;
	int id;
	bool begInited;
public:
	virtual void Close ( int ) = 0; // ( peerthread -> ) call -> thread
protected:
	LegThread ( H323Call * c, LegThread * * p, unsigned _id );
};

#endif
