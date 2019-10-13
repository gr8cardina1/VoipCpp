#ifndef TOTALACTIVERESOURCE_HPP_
#define TOTALACTIVERESOURCE_HPP_
#pragma interface

class TotalActiveResource : public PHTTPResource, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( TotalActiveResource, PHTTPResource )
public:
	TotalActiveResource ( );
protected:
	PString LoadText ( PHTTPRequest & );
	BOOL LoadHeaders ( PHTTPRequest & request );
};

#endif /*TOTALACTIVERESOURCE_HPP_*/
