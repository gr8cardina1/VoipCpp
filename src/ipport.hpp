#ifndef __IPPORT_HPP
#define __IPPORT_HPP
#pragma interface

struct IpPort {
	ss :: string ip;
	int port;
	IpPort ( const ss :: string & i, int p ) : ip ( i ), port ( p ) { }
	bool operator < ( const IpPort & v ) const {
		if ( ip < v.ip )
			return true;
		if ( ip > v.ip )
			return false;
		return port < v.port;
	}
};

typedef std :: map < ss :: string, IpPort, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, IpPort > > > IpPortMap;
typedef std :: set < IpPort, std :: less < IpPort >, __SS_ALLOCATOR < IpPort > > IpPortSet;
typedef std :: map < ss :: string, IpPortMap, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, IpPortMap > > > RecodersMap;

#endif
