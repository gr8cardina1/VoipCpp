#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "mgcp.hpp"
#include "mgcpepstatehandler.hpp"

void EpStateHandler :: handleRequest ( MGCP :: PduVector & ret, const MGCP :: PDU & pdu, const PIPSocket :: Address & /*ia*/ ) {
	makeResponse ( ret, pdu, MGCP :: PDU :: rcUnknownCommand );
}

void EpStateHandler :: handleResponse ( MGCP :: PduVector & /*ret*/, const MGCP :: PDU & /*pdu*/ ) {
	//nado chto-to umnoe delat
}
void EpStateHandler :: sendRinging ( MGCP :: PduVector & /*ret*/, int /*localRtpPort*/,
	const PIPSocket :: Address & /*localAddr*/, const CodecInfo & /*inCodec*/ ) { }
void EpStateHandler :: sendOk ( MGCP :: PduVector & /*ret*/, int /*localRtpPort*/, const PIPSocket :: Address & /*localAddr*/,
	const CodecInfo & /*inCodec*/, unsigned /*telephoneEventsPayloadType*/ ) { }
