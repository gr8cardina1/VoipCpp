#ifndef __H245HANDLER_HPP
#define __H245HANDLER_HPP
#pragma interface

class Session;
class RTPStat;
class CommonCallDetailsBaseOut;

struct RequestElement {
	H245 :: DataType dataType;
	ss :: string recodeFrom, recodeTo;
	int rtpCodec, rtpFrames;
	RequestElement ( const H245 :: DataType & d );
};

std :: ostream & operator<< ( std :: ostream & os, const RequestElement & e );

class H245Handler : public Allocatable < __SS_ALLOCATOR > {
public:
	typedef std :: map < ss :: string, FastStartElement, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, FastStartElement > > > FastStartMap;
	typedef std :: map < int, RequestElement, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, RequestElement > > > RequestMap;
private:
	volatile bool & differentCodec;
	PIPSocket :: Address from, to;
	PMutex mut;
	Session * rtpSession;
	RTPStat * stats;
	CodecInfo codec;
	bool direct;
	int fromNat;
	bool toNat;
	SourceData source;
	bool inAllCodecs, outAllCodecs;
	CodecInfoSet inAllowedCodecs, outAllowedCodecs;
	int outPeer;
	bool inAnswerToRTDR, outAnswerToRTDR;
	bool callerBreak, calledBreak;
	RequestMap inChannelRequests, outChannelRequests;
	bool dropTele;
	FastStartMap originalFastStartIn, originalFastStartOut;
	bool handleRequest ( H245 :: MultimediaSystemControlMessage & msg, bool fromCaller, bool & changeTarget );
	// Task: to handle the given H.245 request

	bool handleResponse ( H245 :: ResponseMessage & response, bool fromCaller );
	// Task: to handle the given H.245 response

	void handleIndication ( H245 :: IndicationMessage & indication, bool fromCaller );
	// Task: to handle the given H.245 indication
	bool handleOpenLogicalChannel ( H245 :: OpenLogicalChannel & olc, bool fromCaller );
	bool handleOpenLogicalChannelAck ( H245 :: OpenLogicalChannelAck & msg, bool fromCaller );
	void handleOpenLogicalChannelConfirm ( H245 :: OpenLogicalChannelConfirm & msg, bool fromCaller );
	void handleOpenLogicalChannelReject ( H245 :: OpenLogicalChannelReject & msg, bool fromCaller );
	void handleRTDR ( H245 :: MultimediaSystemControlMessage & msg, bool fromCaller, bool & changeTarget );
	void handleCapabilitySet ( H245 :: TerminalCapabilitySet & caps, bool /*FromCaller*/ );

	bool admissibleIP ( bool caller, PIPSocket :: Address & ip );
	// Task: Used on logical channel procedures to check if the ip address used
	// for the logical channel is admissible. The admissible ips are the
	// ip used for RAS addresses or the ip of the call signal addresses
	// of the indicated endpoint (caller or called).
	// If the endpoint is not registered, the given address is ammissible
	// only if the endpoint id is the bogus id and the ip is external to
	// the network covered by the proxy.

	bool admissiblePort ( WORD port );
	// Task: Used on open logical channel procedures to check if the port number
	// used for the logical channel is admissible. Only non system ports
	// are admissible.
	PIPSocket :: Address getLocalAddress ( bool callerSide );
	// Task: return the address of the interface to the caller host or the called host.
	bool checkAddr ( H245 :: TransportAddress & addr, H245 :: UnicastAddress_iPAddress * & ip,
		PIPSocket :: Address & ipAddr, bool fromCaller );
	bool checkCap ( const CodecInfo & codec, bool fromCaller, H245 :: AudioCapability & cap ) const;
	void removeUnsupported ( FastStartElementVector & fs ) const;
	void removeReverseUnsupported ( FastStartElementVector & fs ) const;
	void getRecodesMapIn ( FastStartElementVector & in );
	void getRecodesMapOut ( FastStartElementVector & out );
	void getNoRecodesMapIn ( FastStartElementVector & fs );
	void getNoRecodesMapOut ( FastStartElementVector & fs );
public:
	H245Handler ( volatile bool & differentCodec, const CommonCallDetailsBaseOut & call, RTPStat * st, bool directRTP );
	bool handleMesg ( H245 :: MultimediaSystemControlMessage & mesg, bool fromCaller, bool & changeTarget );
	// Task: to handle the given H.245 message
	void setFrom ( const PIPSocket :: Address & f );
	PIPSocket :: Address getFrom ( ) const;
	void setTo ( const PIPSocket :: Address & t, bool tn );
	PIPSocket :: Address getTo ( ) const;
	void setOutPeer ( int op, bool oa, const CodecInfoSet & oac, bool oatrtdr );
	~H245Handler ( );
	H225 :: ArrayOf_Asn_OctetString handleFastStartNew ( const H225 :: ArrayOf_Asn_OctetString & fastStart );
	bool handleFastStartResponseNew ( H225 :: ArrayOf_Asn_OctetString & fastStart );
	Session * getRtpSession ( );
	const CodecInfo & getCodec ( );
	bool getCallerBreak ( ) const;
	bool getCalledBreak ( ) const;
};
#endif
