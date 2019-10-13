#ifndef MGCPANSWERLEG_HPP_
#define MGCPANSWERLEG_HPP_
#pragma interface

class MgcpAnswerLegThread : public AnswerLegThread, public EpThreadHandler {
	PCLASSINFO ( MgcpAnswerLegThread, AnswerLegThread )
	CommonCallDetails ccd;
	ss :: string gw, ep;
	struct Data;
	Data * p;
	bool detached;
	void detach ( );
	void detachLocal ( );
	bool iteration ( );
	bool ended ( ) const;
	void shutDownLeg ( );
	void init ( );
	void peerAlertedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr, const CodecInfo & inCodec,
		const CodecInfo & outCodec );
	void peerConnectedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr, const CodecInfo & inCodec,
		const CodecInfo & outCodec );
	void peerOnHoldLeg ( int /*level*/ ) { } //stub
	void peerOnHoldOKLeg ( ) { } //stub
	void peerDtmfLeg ( const DTMF :: Relay & /*r*/ ) { } // stub
	void putMessage ( const PeerMessage & m );
	~MgcpAnswerLegThread ( );
public:
	MgcpAnswerLegThread ( H323Call * c, LegThread * * p, unsigned i, const CommonCallDetails & d, const ss :: string & g,
		const ss :: string & e );
};

#endif /*MGCPANSWERLEG_HPP_*/
