#ifndef __RTPREQUEST_HPP
#define __RTPREQUEST_HPP
#pragma interface

enum RTPReqType {
	rqAlloc,
	rqFree,
	rqSetSendAddress,
	rqInit,
	rqGetTimeout,
	rqEnableLog,
	rqSetTo,
	rqSetFrom,
	rqSetTelephoneEventsPayloadType
};
struct RTPRequest {
	RTPReqType type;
	int data [ sizeof ( RTPStat ) * 4 / sizeof ( int ) ];
	RTPRequest ( RTPReqType t ) : type ( t ) {
		memset ( data, 0, sizeof ( data ) );
	}
};
std :: ostream & operator<< ( std :: ostream & os, const RTPRequest & r );
#endif
