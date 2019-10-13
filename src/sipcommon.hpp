#ifndef __SIPCOMMON_HPP
#define __SIPCOMMON_HPP
#pragma interface

namespace SIP {
	class MediaFormat;
	class SessionDescription;
	class MediaDescription;
	class PDU;
	typedef std :: vector < MediaDescription, __SS_ALLOCATOR < MediaDescription > > MediaDescriptionVector;
}
class CodecInfoVector;

CodecInfo getCodec ( const SIP :: MediaFormat & format );
ss :: string toHex ( const ss :: string & s );
bool getIpFromVia ( const ss :: string & via, ss :: string & ip );
bool getPortFromVia ( const ss :: string & via, int & port );
void getIncomingCodecsFromSDP ( const SIP :: SessionDescription & sdp, CodecInfoVector & incomingCodecs );
void getIncomingCodecsFromSDP ( const SIP :: SessionDescription & sdp, const StringVector& inpeerVector, CodecInfoVector * incomingCodecs );
void addFormat ( SIP :: MediaDescription & media, const CodecInfo & c );
ss :: string changeCSeq ( const ss :: string & oldCSeq, const ss :: string & method );
unsigned allocTelephoneEventsPayload ( const SIP :: MediaDescriptionVector & mdv );
void clearContent ( SIP :: PDU & mesg );
#endif
