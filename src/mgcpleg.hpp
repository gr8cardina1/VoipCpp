#ifndef MGCPLEG_HPP_
#define MGCPLEG_HPP_
#pragma interface
class MgcpLeg : public Leg, public EpOriginateThreadHandler {
	RecodeInfoVector inRecodes;
	ss :: string gw, ep;
	struct Data;
	Data * p;
	bool detached;
	bool tryChoice ( );
	bool initChoice ( );
	bool iteration ( );
	void closeChoice ( );
	bool ended ( ) const;
	void wakeUp ( );
	void sdpAck ( const SDP :: SessionDescription & sdp );
	void connected ( );
	void detach ( );
	~MgcpLeg ( );
	bool sdpAckLocal ( const SDP :: SessionDescription & sdp );
	bool connectedLocal ( );
	bool detachLocal ( );
	bool peerOnHold ( int /*level*/ ) { return true; } // stub
	bool peerOnHoldOK ( ) { return true; } // stub
	bool peerDtmf ( const DTMF :: Relay & /*r*/ ) { return true; } //stub
public:
	MgcpLeg ( OriginateLegThread * t, CommonCallDetails & c, const RecodeInfoVector & inCodecs,
		const ss :: string & g, const ss :: string & e );
};
#endif /*MGCPLEG_HPP_*/
