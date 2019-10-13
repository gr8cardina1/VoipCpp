#ifndef SDPCOMMON_HPP_
#define SDPCOMMON_HPP_
#pragma interface

CodecInfo getCodec ( const SDP :: MediaFormat & format, unsigned ptime );
ss :: string addFormat ( SDP :: MediaDescription & media, const CodecInfo & c );
void addTelephoneEvents ( SDP :: MediaDescription & media, unsigned payloadType );
unsigned getTelephoneEventsPayloadType ( const SDP :: MediaDescription * media );
void getIncomingCodecsFromSDP ( const SDP :: SessionDescription & sdp, CodecInfoVector & incomingCodecs );

#endif /*SDPCOMMON_HPP_*/
