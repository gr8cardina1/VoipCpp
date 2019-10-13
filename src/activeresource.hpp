#ifndef ACTIVERESOURCE_HPP_
#define ACTIVERESOURCE_HPP_
#pragma interface

class ActiveResource : public PHTTPResource, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( ActiveResource, PHTTPResource )
public:
	ActiveResource ( );
protected:
	PString LoadText ( PHTTPRequest & request );
	BOOL LoadHeaders ( PHTTPRequest & request );
};

class ActiveResourceCSV : public PHTTPResource, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( ActiveResourceCSV, PHTTPResource )
public:
	ActiveResourceCSV ( );
protected:
	PString LoadText ( PHTTPRequest & request );
	BOOL LoadHeaders ( PHTTPRequest & request );
};

#endif /*ACTIVERESOURCE_HPP_*/
