#ifndef AFTERTASK_HPP_
#define AFTERTASK_HPP_
#pragma interface
class AfterTask : public Allocatable < __SS_ALLOCATOR > {
	AfterTask ( const AfterTask & );
	AfterTask & operator= ( const AfterTask & );
public:
	AfterTask ( );
	virtual ~AfterTask ( );
};

#endif /*AFTERTASK_HPP_*/
