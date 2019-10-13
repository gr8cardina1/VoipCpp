#ifndef IPINPEERS_HPP_
#define IPINPEERS_HPP_
#pragma interface

class MySQL;

class IpInPeers : boost :: noncopyable {
	class Impl;
	Impl * impl;
public:
	IpInPeers ( );
	~IpInPeers ( );
	void load ( MySQL & m, const std :: tr1 :: function < bool ( int ) > & hasPeer );
	int getInPeer ( const ss :: string & ip, const ss :: string & callingDigits,
		const ss :: string & dialedDigits, ss :: string & realDigits, int & fromNat,
		ss :: string & inPrefix, int & retCode ) const;
	bool validInPeerAddress ( const ss :: string & addr ) const;
};

#endif /*IPINPEERS_HPP_*/
