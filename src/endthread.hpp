#ifndef __ENDTHREAD_HPP
#define __ENDTHREAD_HPP
#pragma interface

class Conf;
class EndThread : public PThread, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( EndThread, PThread )
public:
	EndThread ( );
	~EndThread ( );
private:
	void Main ( );
};
#endif
