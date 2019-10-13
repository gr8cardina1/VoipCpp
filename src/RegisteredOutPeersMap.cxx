#pragma implementation
#include "ss.hpp"
#include <ptlib.h>
#include "ipport.hpp"
#include "RegisteredOutPeersMap.hpp"
#include <ptlib/svcproc.h>

RegisteredOutPeersMap :: RegisteredOutPeersMap ( ) {
}

RegisteredOutPeersMap :: ~RegisteredOutPeersMap ( ) {
}

void RegisteredOutPeersMap :: addPeer ( const RegisteredOutPeer & peer ) {
	int peerId = peer.getPeerId ( );
	ss :: string ipAddr = peer.getIpAddr ( );
	IpPort ipPort ( ipAddr, peer.getPort ( ) );
	_peerIdToOutPeerMap [ peerId ] = peer;
	_ipPortToOutPeerMap [ ipPort ] = peer;
}

const RegisteredOutPeer * RegisteredOutPeersMap :: getPeer ( int peerId ) const {
	PeerIdToOutPeerMap :: const_iterator it = _peerIdToOutPeerMap.find ( peerId );
	if ( it != _peerIdToOutPeerMap.end ( ) )
		return & it -> second;
	else
		return NULL;
}

const RegisteredOutPeer * RegisteredOutPeersMap :: getPeer ( const IpPort & ipPort ) const {
	IpPortToOutPeerMap :: const_iterator it = _ipPortToOutPeerMap.find ( ipPort );
	if ( it != _ipPortToOutPeerMap.end ( ) )
		return & it -> second;
	else
		return NULL;
}

void RegisteredOutPeersMap :: removePeer ( int peerId ) {
	const RegisteredOutPeer * peer = getPeer ( peerId );
	if ( peer == NULL )
		return;
	IpPort ipPort ( peer -> getIpAddr ( ), peer -> getPort ( ) );
	_ipPortToOutPeerMap.erase ( ipPort );
	_peerIdToOutPeerMap.erase ( peerId );
}

void RegisteredOutPeersMap :: removePeer ( const IpPort & ipPort ) {
	const RegisteredOutPeer * peer = getPeer ( ipPort );
	if ( peer == NULL )
		return;
	int peerId = peer -> getPeerId ( );
	_peerIdToOutPeerMap.erase ( peerId );
	_ipPortToOutPeerMap.erase ( ipPort );
}

void RegisteredOutPeersMap :: updatePeer ( int peerId, const ss :: string & ip, int port, int timeout,
	const ss :: string & suffix, PTime endLease, bool h ) {
	removePeer ( peerId );
	RegisteredOutPeer newPeer ( peerId, ip, port, timeout, suffix, endLease, h );
	addPeer ( newPeer );
}

void RegisteredOutPeersMap :: eraseExpired ( PTime now ) {
	for ( PeerIdToOutPeerMap :: iterator it = _peerIdToOutPeerMap.begin ( ); it != _peerIdToOutPeerMap.end ( ); ) {
		if ( it -> second.isExpired ( now ) ) {
			PSYSTEMLOG ( Info, "removing registered outpeer " << it -> second.getPeerId ( ) );
			IpPort ipPort ( it -> second.getIpAddr ( ), it -> second.getPort ( ) );
			PeerIdToOutPeerMap :: iterator it2 = it ++;
			_ipPortToOutPeerMap.erase ( ipPort );
			_peerIdToOutPeerMap.erase ( it2 );
		} else {
			++ it;
		}
	}
}
