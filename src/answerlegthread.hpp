#ifndef __ANSWERLEGTHREAD_HPP
#define __ANSWERLEGTHREAD_HPP
#pragma interface

namespace DTMF {
struct Relay;
}

class AnswerLegThread : public LegThread {
	PCLASSINFO ( AnswerLegThread, LegThread )
	CommonCallDetails & common;
	bool endInited;
	PTime endTime;
	virtual void shutDownLeg ( ) = 0; //local, odin raz, posle vizova dolgno stat ended ( ) dolgno stat true;
	virtual void peerAlertedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, const CodecInfo & outCodec ) = 0; //local
	virtual void peerConnectedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, const CodecInfo & outCodec ) = 0; //local
	virtual void peerOnHoldLeg ( int level ) = 0; //local
	virtual void peerOnHoldOKLeg ( ) = 0; //local
	virtual void peerDtmfLeg ( const DTMF :: Relay & r ) = 0; //local
	virtual void init ( ) = 0; //local
	virtual bool ended ( ) const = 0; //local
	virtual bool iteration ( ) = 0; //local
	void released ( Q931 :: CauseValues cause ); // = 0
	void Close ( int ); // ( peerthread -> ) call -> thread
	void closeLocal ( int ); // local
	void peerAlertedLocal ( int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, const CodecInfo & outCodec ); // local
	void peerConnectedLocal ( int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, const CodecInfo & outCodec, unsigned telephoneEventsPayloadType ); // local
	void peerOnHoldLocal ( int level ); // local
	void peerOnHoldOKLocal ( ); // local
	void peerDtmfLocal ( const DTMF :: Relay & r ); // local
protected:
	typedef std :: tr1 :: function < void ( ) > PeerMessage;
private:
	virtual void putMessage ( const PeerMessage & m ) = 0; // nonlocal
protected:
	void shutDown ( int cause = 0 ); // mogno vizivat neskolko raz, tolko iz localnogo contexta, posle vizova ended ( ) == true
					 // pri cause != 0 peredaet ego peeru
	void Main ( );
	bool admissibleIP ( PIPSocket :: Address & ip );
	~AnswerLegThread ( );
	AnswerLegThread ( H323Call * c, LegThread * * p, unsigned i, CommonCallDetails & com );
public:
	void peerAlerted ( int localRtpPort, const PIPSocket :: Address & localAddr,
		const CodecInfo & inCodec, const CodecInfo & outCodec ); // peerthread -> call -> thread
	void peerConnected ( int localRtpPort, const PIPSocket :: Address & localAddr, // peerthread -> call -> thread
		const CodecInfo & inCodec, const CodecInfo & outCodec, unsigned telephoneEventsPayloadType );
	void peerOnHold ( int level );
	void peerOnHoldOK ( );
	void peerDtmf ( const DTMF :: Relay & r );
};

#endif
