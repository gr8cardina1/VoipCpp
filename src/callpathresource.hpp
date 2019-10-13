#ifndef CALLPATHRESOURCE_HPP_
#define CALLPATHRESOURCE_HPP_
#pragma interface

class CallPathResource : public PHTTPResource, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( CallPathResource, PHTTPResource )
public:
	CallPathResource ( );
protected:
	PString LoadText ( PHTTPRequest & request );
	BOOL LoadHeaders ( PHTTPRequest & request );
};

#endif /*CALLPATHRESOURCE_HPP_*/
