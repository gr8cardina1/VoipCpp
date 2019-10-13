#ifndef SIPANSWERLEG2_HPP_
#define SIPANSWERLEG2_HPP_
#pragma interface

namespace SIP2 {
class ServerTransactionId;
}

class SipAnswerLegThread2 : public AnswerLegThread, public SIP2 :: AnswerHandler {
	PCLASSINFO ( SipAnswerLegThread2, AnswerLegThread )
	CommonCallDetails ccd;
	PTime startTime;
	struct Data;
	Data * p;
	unsigned expires;
	unsigned lastCseq;
	bool oked;
	bool detached;
	void disconnectReceived ( int c );
	void disconnectReceivedLocal ( int c );
	void cancelReceived ( );
	void cancelReceivedLocal ( );
	void onholdReceived ( int level, int port, const ss :: string & addr );
	void onholdReceivedLocal ( int level, int port, const ss :: string & addr );
	void dtmfRelayReceived ( const DTMF :: Relay & r );
	void dtmfRelayReceivedLocal ( const DTMF :: Relay & r );
	bool iteration ( );
	bool ended ( ) const;
	void shutDownLeg ( );
	void init ( );
	void peerAlertedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr, const CodecInfo & inCodec,
		const CodecInfo & outCodec );
	void peerConnectedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr, const CodecInfo & inCodec,
		const CodecInfo & outCodec );
	void peerOnHoldLeg ( int level );
	void peerOnHoldOKLeg ( ) { } //stub
	void peerDtmfLeg ( const DTMF :: Relay & r );
	void putMessage ( const PeerMessage & m );
	~SipAnswerLegThread2 ( );
public:
	SipAnswerLegThread2 ( H323Call * c, LegThread * * p, unsigned i, const CommonCallDetails & d,
		const SIP2 :: ServerTransactionId & id, unsigned expires );
};

#endif /*SIPANSWERLEG2_HPP_*/
