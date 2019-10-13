#ifndef MGCPSIGNALPARSER_HPP_
#define MGCPSIGNALPARSER_HPP_
#pragma interface

namespace MGCP {

class EventParameter : public Allocatable < __SS_ALLOCATOR > {
public:
	typedef std :: vector < Pointer < EventParameter >, __SS_ALLOCATOR < Pointer < EventParameter > > > List;
private:
	ss :: string nameOrValue;
	List parameters;
public:
	EventParameter ( const ss :: string & n ) : nameOrValue ( n ) { }
	const ss :: string & getName ( ) const {
		return nameOrValue;
	}
	void add ( const Pointer < EventParameter > & p ) {
		parameters.push_back ( p );
	}
	const List & getParameters ( ) const {
		return parameters;
	}
	void clear ( ) {
		parameters.clear ( );
	}
	void printOn ( std :: ostream & os ) const;
};

inline std :: ostream & operator<< ( std :: ostream & os, const EventParameter :: List & parameters ) {
	bool first = true;
	for ( EventParameter :: List :: const_iterator i = parameters.begin ( ); i != parameters.end ( ); ++ i ) {
		if ( first )
			first = false;
		else
			os << ", ";
		( * i ) -> printOn ( os );
	}
	return os;
}

class EventName {
	ss :: string packageName;
	ss :: string eventId;
	ss :: string connectionId;
public:
	EventName ( const ss :: string & p, const ss :: string & e, const ss :: string & c = ss :: string ( ) ) :
		packageName ( p ), eventId ( e ), connectionId ( c ) { }
	const ss :: string & getPackage ( ) const {
		return packageName;
	}
	const ss :: string & getEvent ( ) const {
		return eventId;
	}
	const ss :: string & getConnection ( ) const {
		return connectionId;
	}
	void printOn ( std :: ostream & os ) const {
		if ( ! packageName.empty ( ) )
			os << packageName << '/';
		os << eventId;
		if ( ! connectionId.empty ( ) )
			os << '@' << connectionId;
	}
};

class SignalRequest {
	EventName name;
	EventParameter :: List parameters;
public:
	SignalRequest ( const EventName & n ) : name ( n ) { }
	void add ( const Pointer < EventParameter > & p ) {
		parameters.push_back ( p );
	}
	void clear ( ) {
		parameters.clear ( );
	}
	const EventParameter:: List & getParameters ( ) const {
		return parameters;
	}
	const EventName & getName ( ) const {
		return name;
	}
	void printOn ( std :: ostream & os ) const {
		name.printOn ( os );
		if ( ! parameters.empty ( ) )
			os << '(' << parameters << ')';
	}
};

typedef std :: vector < SignalRequest, __SS_ALLOCATOR < SignalRequest > > SignalRequestVector;

SignalRequestVector parseSignalRequests ( const ss :: string & s );

inline std :: ostream & operator<< ( std :: ostream & os, const SignalRequest & s ) {
	s.printOn ( os );
	return os;
}

}

#endif /*MGCPSIGNALPARSER_HPP_*/
