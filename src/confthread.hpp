#ifndef __CONFTHREAD_HPP
#define __CONFTHREAD_HPP
#pragma interface

class Conf;
class ConfThread : public PThread, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( ConfThread, PThread )
public:
	ConfThread ( );
	~ConfThread ( );
private:
	void Main ( );
};
#endif
