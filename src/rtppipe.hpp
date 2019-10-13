#ifndef __RTPPIPE_HPP
#define __RTPPIPE_HPP
#pragma interface

class RTPRequest;
class RTPPipe : public Allocatable < __SS_ALLOCATOR > {
	PPipeChannel pipe;
	PMutex mut;
public:
	RTPPipe ( );
	~RTPPipe ( );
	bool send ( RTPRequest & r, int retCount = 0 );
};
#endif
