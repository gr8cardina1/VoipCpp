#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <boost/noncopyable.hpp>
#include <tr1/functional>
#include "ipinpeers.hpp"
#include "pointer.hpp"
#include <stdexcept>
#include "mysql.hpp"
#include <ptlib.h>
#include <ptlib/svcproc.h>
#include "translateipmask.hpp"
#include "findprefix.hpp"
#include "findmapelement.hpp"

class IpInPeers :: Impl : public Allocatable < __SS_ALLOCATOR > {
	struct PrefToPeer {
		int peer;
		ss :: string realPrefix;
		int fromNat;
		bool m_isBlockAnum;
		int m_replayCode;
		PrefToPeer ( int p, const ss :: string & rp, int fn,  bool isBlockAnum, int replayCode ) : 
		    peer ( p ), realPrefix ( rp ), fromNat ( fn ), m_isBlockAnum(isBlockAnum), m_replayCode(replayCode) { }
	};
	typedef std :: map < ss :: string, PrefToPeer, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, PrefToPeer > > > PrefToPeerMap;
	typedef std :: map < ss :: string, PrefToPeerMap, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, PrefToPeerMap > > > AniPrefToPeerMap;
	typedef std :: map < ss :: string, AniPrefToPeerMap, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, AniPrefToPeerMap > > > IpToPeerMap;
	IpToPeerMap ipToPeer;
	static int getInPeer ( const AniPrefToPeerMap & amap, const ss :: string & callingDigits,
		const ss :: string & dialedDigits, ss :: string & realDigits, int & fromNat,
		ss :: string & inPrefix, int & retCode );
	int getInPeerInt ( const ss :: string & ip, const ss :: string & callingDigits,
		const ss :: string & dialedDigits, ss :: string & realDigits, int & fromNat,
		ss :: string & inPrefix, int & retCode ) const;
public:
	void load ( MySQL & m, const std :: tr1 :: function < bool ( int ) > & hasPeer );
	bool validInPeerAddress ( const ss :: string & addr ) const {
		return ipToPeer.count ( addr );
	}
	int getInPeer ( const ss :: string & ip, const ss :: string & callingDigits,
		const ss :: string & dialedDigits, ss :: string & realDigits, int & fromNat,
		ss :: string & inPrefix, int & retCode ) const;
};

void IpInPeers :: Impl :: load ( MySQL & m, const std :: tr1 :: function < bool ( int ) > & hasPeer ) {
	MyResult r ( m.getQuery ( "select ip, anum, prefix, uid, realPrefix, nat - 1, isBlockANum, responseCode "
		"from InIPAddresses" ) );
	union PrefToPeerData {
		const char * p [ 8 ];
		struct {
			const char * ip;
			const char * anum;
			const char * prefix;
			const char * uid;
			const char * realPrefix;
			const char * nat;
	                const char * isBlockAnum;
        		const char * replayCode;
		} n;
	};
	while ( PrefToPeerData * p = reinterpret_cast < PrefToPeerData * > ( r.fetchRow ( ) ) ) {
		int id = std :: atoi ( p -> n.uid );
		if ( ! hasPeer ( id ) )
			continue;
		ss :: string anum = p -> n.anum;
		ss :: string :: size_type pos = anum.find ( '.' );
		if ( pos != ss :: string :: npos )
			anum.erase ( pos );
		PrefToPeerMap :: value_type vt ( p -> n.prefix,
			PrefToPeer ( std :: atoi ( p -> n.uid ), p -> n.realPrefix, std :: atoi ( p -> n.nat ),
			p -> n.isBlockAnum [ 0 ] == 'y', std :: atoi ( p -> n.replayCode ) ) );
		StringVector ips = translateIpMask ( p -> n.ip );
		for ( StringVector :: const_iterator i = ips.begin ( ); i != ips.end ( ); ++ i ) {
			PSYSTEMLOG ( Info, "Adding ipToPeer (" << * i << ':' << anum << ':' <<
				p -> n.prefix << "):(" << p -> n.uid << ',' << p -> n.realPrefix << ',' << p -> n.nat <<
				',' << p -> n.isBlockAnum << ',' << p -> n.replayCode << ')' );
			ipToPeer [ * i ] [ anum ].insert ( vt );
		}
	}
}

int IpInPeers :: Impl :: getInPeer ( const AniPrefToPeerMap & amap, const ss :: string & callingDigits,
	const ss :: string & dialedDigits, ss :: string & realDigits, int & fromNat, ss :: string & inPrefix,
	int & retCode ) {
	AniPrefToPeerMap :: const_iterator an = findPrefix ( amap, callingDigits );
	if ( an == amap.end ( ) )
		return 0;
	const PrefToPeerMap & prefToPeers = an -> second;
	PrefToPeerMap :: const_iterator pi = findPrefix ( prefToPeers, dialedDigits );
	if ( pi == prefToPeers.end ( ) )
		return 0;
	realDigits = pi -> second.realPrefix + dialedDigits.substr ( pi -> first.size ( ) );
	fromNat = pi -> second.fromNat;
	inPrefix = pi -> first;
        if ( ! pi -> second.m_isBlockAnum )
        	return pi -> second.peer;
	retCode = pi -> second.m_replayCode;
	return 0;
}

int IpInPeers :: Impl :: getInPeerInt ( const ss :: string & ip, const ss :: string & callingDigits,
	const ss :: string & dialedDigits, ss :: string & realDigits, int & fromNat, ss :: string & inPrefix,
	int & retCode ) const {
	if ( const AniPrefToPeerMap * amap = findMapElement ( ipToPeer, ip ) )
		return getInPeer ( * amap, callingDigits, dialedDigits, realDigits, fromNat, inPrefix, retCode );
	return 0;
}

int IpInPeers :: Impl :: getInPeer ( const ss :: string & ip, const ss :: string & callingDigits,
	const ss :: string & dialedDigits, ss :: string & realDigits, int & fromNat, ss :: string & inPrefix,
	int & retCode ) const {
	if ( int r = getInPeerInt ( ip, callingDigits, dialedDigits, realDigits, fromNat, inPrefix, retCode ) )
		return r;
	if ( ip.empty ( ) )
		return 0;
	return getInPeerInt ( ss :: string ( ), callingDigits, dialedDigits, realDigits, fromNat, inPrefix, retCode );
}

IpInPeers :: IpInPeers ( ) : impl ( new Impl ) { }

IpInPeers :: ~IpInPeers ( ) {
	delete impl;
}

void IpInPeers :: load ( MySQL & m, const std :: tr1 :: function < bool ( int ) > & hasPeer ) {
	impl -> load ( m, hasPeer );
}

bool IpInPeers :: validInPeerAddress ( const ss :: string & addr ) const {
	return impl -> validInPeerAddress ( addr );
}

int IpInPeers :: getInPeer ( const ss :: string & ip, const ss :: string & callingDigits,
	const ss :: string & dialedDigits, ss :: string & realDigits, int & fromNat,
	ss :: string & inPrefix, int & retCode ) const  {
	return impl -> getInPeer ( ip, callingDigits, dialedDigits, realDigits, fromNat, inPrefix, retCode );
}
