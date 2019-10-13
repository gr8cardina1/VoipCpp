#ifndef __H323COMMON_HPP
#define __H323COMMON_HPP
#pragma interface

const bool multipleCalls = false;
extern ss :: string h225ProtocolID;

CodecInfo getCodec ( const H245 :: AudioCapability & audioData );
CodecInfo getCodec ( const H245 :: DataType & dataType );
class CommonCallDetails;
struct FastStartElement {
	H245 :: OpenLogicalChannel open;
	CodecInfo codec;
	int changeFrames, outChangeFrames;
	CodecInfo recodeTo;
	FastStartElement ( const H245 :: OpenLogicalChannel & o, const H245 :: AudioCapability & c, int ch = 0 );
	FastStartElement ( ) { }
};
typedef std :: vector < FastStartElement, __SS_ALLOCATOR < FastStartElement > > FastStartElementVector;
class PTCPSocket;
bool tryNextChoiceIterationH323 ( CommonCallDetails & common, PTCPSocket * & calledSocket );
void setIPAddress ( H245 :: UnicastAddress_iPAddress & ip, const PIPSocket :: Address & addr, int port );
void encodeH225IntoQ931 ( const H225 :: H323_UserInformation & uuField, Q931 & mesg );
void sendMesgToDest ( const Q931 & mesg, const H225 :: H323_UserInformation * uuField,
	PTCPSocket * destination, const CommonCallDetails & common );
void sendH245MesgToDest ( const ss :: string & s, const H245 :: MultimediaSystemControlMessage & mesg,
	PTCPSocket * destination, bool forceDebug );
void translateSetup ( const CommonCallDetails & common, const Q931 & orig, const H225 :: CallIdentifier & id,
	Q931 & mesg, H225 :: H323_UserInformation & uuField, const H225 :: TransportAddress & sourceCallSigAddr,
	const H225 :: TransportAddress & destCallSigAddr );
template < class T > void handleMultipleCalls ( T & uuie ) {
	uuie.includeOptionalField ( T :: e_multipleCalls );
	uuie.get_multipleCalls ( ) = multipleCalls;
	uuie.includeOptionalField ( T :: e_maintainConnection );
	uuie.get_maintainConnection ( ) = multipleCalls;
}
void sendReleaseComplete ( PTCPSocket * sock, CommonCallDetails & common, const H225 :: CallIdentifier & id, int ref,
	bool fromCaller );
void dropNonStandard ( H225 :: H323_UserInformation & uuField );
template < class T > void handleCallId ( T & uuie, const H225 :: CallIdentifier & id ) {
	if ( uuie.hasOptionalField ( T :: e_callIdentifier ) )
		return;
	uuie.includeOptionalField ( T :: e_callIdentifier );
	uuie.get_callIdentifier ( ) = id;
}
template < class T > void dropTokens ( T & uuie ) {
	uuie.removeOptionalField ( T :: e_tokens );
	uuie.removeOptionalField ( T :: e_cryptoTokens );
}
bool admissiblePort ( int port );
void getControlAddr ( H245 :: H2250LogicalChannelParameters & h225p, PIPSocket :: Address & addr,
	int & port, H245 :: UnicastAddress_iPAddress * & ip );
void getMediaAddr ( H245 :: H2250LogicalChannelParameters & h225p, PIPSocket :: Address & addr,
	int & port, H245 :: UnicastAddress_iPAddress * & ip );
Q931 :: CauseValues getCause ( const Q931 & mesg, const H225 :: H323_UserInformation * uuField );
Q931 readMsg ( PTCPSocket * socket );
void getInPrice ( const H225 :: H323_UU_PDU & pdu, CommonCallDetails & common );
bool getCallID ( const H225 :: Setup_UUIE & setup, H225 :: CallIdentifier & id );
void setFrames ( H245 :: AudioCapability & audioData, int f );
bool setCodec ( H245 :: AudioCapability & audioData, const ss :: string & c );
void addFastStartFromIn ( H225 :: ArrayOf_Asn_OctetString & v, const PIPSocket :: Address & local,
	int port, const CodecInfo & inCodec, int & channel, bool answer );
void addFastStartFromOut ( H225 :: ArrayOf_Asn_OctetString & v, const PIPSocket :: Address & local,
	int port, const CodecInfo & outCodec, bool answer );
void appendBack ( H225 :: ArrayOf_Asn_OctetString & to, const H225 :: ArrayOf_Asn_OctetString & from );
bool getAddrFromGK ( const PIPSocket :: Address & gk, const ss :: string & digits, PIPSocket :: Address & ip, int & port );
void getIncomingCodecsFromSetup ( const H225 :: H323_UU_PDU & pdu, CodecInfoVector & incomingCodecs, PIPSocket :: Address & addr, int & port );
int getRtpCodec ( H245 :: AudioCapability & audioData );
int getRtpCodec ( const CodecInfo & c );
void changeDataTypeAndFramesIn ( FastStartElement & e );
void changeDataTypeAndFramesOut ( FastStartElement & e );
void makeAlertingFromCallProceeding ( Q931 & mesg, H225 :: H323_UU_PDU_h323_message_body & body );
ss :: string makeTpkt ( const ss :: string & s );
#endif
