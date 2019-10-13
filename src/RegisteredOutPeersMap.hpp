#ifndef _REGISTEREDOUTPEERSMAP_HPP_
#define _REGISTEREDOUTPEERSMAP_HPP_
#pragma interface

class RegisteredOutPeer {
public:
	RegisteredOutPeer ( int peerId, const ss :: string & ip, int port, int to, const ss :: string & suf,
		PTime endLease, bool h ) : _peerId ( peerId ), _ipAddr ( ip ), suffix ( suf ), _port ( port ), timeout ( to ),
		_endLease ( endLease ), h323 ( h ) { }
	RegisteredOutPeer ( ) { }
	ss :: string getIpAddr ( ) const {
		return _ipAddr;
	}
	ss :: string getSuffix ( ) const {
		return suffix;
	}
	int getPort ( ) const {
		return _port;
	}
	int getTimeout ( ) const {
		return timeout;
	}
	int getPeerId ( ) const {
		return _peerId;
	}
	bool isExpired ( PTime now ) const {
		return _endLease < now;
	}
	bool getH323 ( ) const {
		return h323;
	}
private:
	int _peerId;
	ss :: string _ipAddr, suffix;
	int _port, timeout;
	PTime _endLease;
	bool h323;
};

class RegisteredOutPeersMap {
public:
	RegisteredOutPeersMap ( );
	virtual ~RegisteredOutPeersMap ( );

	void addPeer ( const RegisteredOutPeer & peer );
	const RegisteredOutPeer * getPeer ( int peerId ) const;
	const RegisteredOutPeer * getPeer ( const IpPort & ipPort ) const;
	void removePeer ( int peerId );
	void removePeer ( const IpPort & ipPort );
	void updatePeer ( int peerId, const ss :: string & ip, int port, int timeout,
		const ss :: string & suffix, PTime endLease, bool h );
	void eraseExpired ( PTime now );

private:
	typedef std :: map < int, RegisteredOutPeer, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, RegisteredOutPeer > > > PeerIdToOutPeerMap;
	typedef std :: map < IpPort, RegisteredOutPeer, std :: less < IpPort >,
		__SS_ALLOCATOR < std :: pair < const IpPort, RegisteredOutPeer > > > IpPortToOutPeerMap;

	PeerIdToOutPeerMap _peerIdToOutPeerMap;
	IpPortToOutPeerMap _ipPortToOutPeerMap; //pomoemu tut multiindex prositsya
};

#endif //_REGISTEREDOUTPEERSMAP_HPP_
