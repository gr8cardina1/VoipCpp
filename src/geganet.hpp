#ifndef __GEGANET_HPP
#define __GEGANET_HPP
#pragma interface

class Geganet {
	int nActive;
	typedef std :: queue < PTime, std :: deque < PTime, __SS_ALLOCATOR < PTime > > > QueueType;
	QueueType rcs;
public:
	bool take ( int timeout, int nChannels );
	void release ( bool connected );
	Geganet ( );
};
#endif
