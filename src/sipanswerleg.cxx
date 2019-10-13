#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "session.hpp"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "rtpsession.hpp"
#include "rtpstat.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "q931.hpp"
#include "pointer.hpp"
#include "codecinfo.hpp"
#include "signallingoptions.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "h323call.hpp"
#include "answercall.hpp"
#include "automutex.hpp"
#include "callcontrol.hpp"
#include "moneyspool.hpp"
#include "sourcedata.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include "legthread.hpp"
#include "answerlegthread.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include "ixcudpsocket.hpp"
#include "sip.hpp"
#include "sipcalldetails.hpp"
#include <queue>
#include "sipanswerleg.hpp"
#include "sipcommon.hpp"
#include "condvar.hpp"
#include "SIPAuthenticateValidator.hpp"
#include "threadmessagequeue.hpp"
#include "sipsignallingthread.hpp"
#include <ptlib/svcproc.h>
#include <cstring>

SipAnswerLegThread :: SipAnswerLegThread ( H323Call * c, LegThread * * p, unsigned i, const SipCallDetails * d,
	const ss :: string & lip ) : AnswerLegThread ( c, p, i, ccd ), ccd ( d -> common ), from_ ( d -> from ),
	to_ ( d -> to ), contact_ ( d -> contact ), /*tag ( toHex ( common.getConfId ( ) ) ),*/
	via ( d -> invite.getMIME ( ).getVia ( ) ), receiver ( 0 ), callId_ ( common.getConfId ( ) ), localIp ( lip ),
	inviteCseq ( d -> invite.getMIME ( ).getCSeq ( ) ), oked ( false ), byeSent ( false ), end ( false ),
	canceled_ ( false ), acked_(false),
	vRecordRoute( d -> invite.getMIME ( ).getRecordRoute ( ) )
   // directRTP_(ccd.getDirectIn ( ) || ccd.curChoice ( ) -> getDirectRTP ( ))
    /*,
	session_(session)*/
	{

//	unsigned payload =	common . getTelephoneEventsPayloadType(  );
//	unsigned payload1 =	ccd . getTelephoneEventsPayloadType(  );
//	PSYSTEMLOG(Info, "SipAnswerLegThread :: SipAnswerLegThread: " << payload << "; 2 = " <<
//							payload1 );

	callId_ = d -> common.getConfId ( );
	tag_ = toHex( callId_ ).substr ( 0, 15 );
	PSYSTEMLOG(Info, "SipAnswerLegThread :: SipAnswerLegThread: " << callId_ << "; toHex ( c.getConfId ( ) ) = " <<
							toHex ( d -> common.getConfId ( ) )<< "; c.getConfId ( ) = " << d -> common.getConfId ( ) );

	PSYSTEMLOG ( Info, "SipAnswerLegThread :: SipAnswerLegThread: from " << from_.str() );
//	tag_ = toHex ( d -> common.getConfId ( ) ).substr ( 0, 15 );
/*

	const char * strVIA = via.data();
	int sizeVIA = via.size();
	ss :: string strProtocol;
	ss :: string newVia;
	for(int i = 0; i < sizeVIA; ++ i)
	{
		if(strVIA[i] == ' ')
		{
			strProtocol = via.substr(0, i);
		}
	}

	newVia = strProtocol;
*/

    strContact1_ = d -> invite.getMIME ( ).getContactString();

    if (strContact1_.empty())
        strContact1_ = d -> invite.getMIME ( ).getRealFrom ( );

	to_.setSipParam ( "tag", tag_ );

	ss :: ostringstream os;
	os << "z9hG4bK." << std :: hex << i;
	branch_ = os.str ( );
	os.str ( "" );
	os << "SIP/2.0/UDP " << localIp << ":5060;branch=" << branch_;
	myVia = os.str ( );
/*
	ss :: string oldVia = via;
	via = myVia;
	via += "\r\n";
	via += oldVia;
*/
	PIPSocket::Address	intrf;

	strURI_ = d -> invite.getURI ( ).str ( );

	if ( sipThread ) {
		sipThread -> GetInterfaceAddress ( intrf );
		intrfIp = static_cast < const char * > ( intrf.AsString ( ) );
	} else
		intrfIp = localIp;

	Resume ( );
}

void SipAnswerLegThread :: init ( ) {
	PSYSTEMLOG(Info, "SipAnswerLegThread :: init");
	receiver = sipThread -> addReceiver ( callId_ );
	end = ! receiver;
}

bool SipAnswerLegThread :: ended ( ) const {
	return end;
}

void SipAnswerLegThread :: shutDownLeg ( ) {
	PSYSTEMLOG(Info, "SipAnswerLegThread :: shutDownLeg");
	end = true;
	if ( ! byeSent ) {
		byeSent = true;
		if ( ! oked ) {
			sendError ( );
			return;
		}

		if( ! canceled_ )
			sendBye ( );
	}
}

SipAnswerLegThread :: ~SipAnswerLegThread ( ) {
	if ( receiver ) {
		sipThread -> removeReceiver ( callId_ );
		receiver = 0;
	}
}

void SipAnswerLegThread :: checkPeerMessages ( ) {
	PeerMessage f;
	while ( true ) {
		{
			AutoMutex am ( receiverMut );
			if ( peerQueue.empty ( ) )
				return;
			f = peerQueue.front ( );
			peerQueue.pop ( );
		}
		f ( );
	}
}


bool SipAnswerLegThread :: iteration ( ) {
	SIP :: PDU mesg;
	bool r = receiveMessage ( mesg );
	checkPeerMessages ( );
	if ( ! r )
		return true;
	return handleMessage ( mesg );
}

bool SipAnswerLegThread :: receiveMessage ( SIP :: PDU & mesg ) {
	return receiver && receiver -> get ( mesg );
}

bool SipAnswerLegThread :: handleMessage ( SIP :: PDU & mesg ) {
	PSYSTEMLOG(Info, "SipAnswerLegThread :: handleMessage. " << mesg.getMethod ( ) );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mBye )
		return handleBye ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mCancel )
		return handleCancel ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mAck )
		return handleAck ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mOptions )
		return handleOptions ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mInvite )
		return handleInvite ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scInformationTrying )
		return handleTrying ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scSuccessfulOK )
		return handleOK ( mesg );

	if ( mesg.getMethod ( ) == SIP :: PDU :: mInfo )
		return handleInfo ( mesg );

 	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) > SIP :: PDU :: scSuccessfulOK )
		return handleError ( mesg );
	PSYSTEMLOG ( Error, "unsupported message" );
	return false;
}

bool SipAnswerLegThread :: sendMessage ( SIP :: PDU & mesg, const ss :: string & addr, int port ) {
	return sipThread -> sendMessage ( mesg, PIPSocket :: Address ( addr.c_str ( ) ), port );
}

bool SipAnswerLegThread :: handleBye ( SIP :: PDU & mesg ) {
	common.setDisconnectCause ( Q931 :: cvNormalCallClearing );
	conf -> replaceReleaseCompleteCause ( common, true );
	call -> released ( common.getDisconnectCause ( ) );
	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "OK" );
	mesg.getMIME().setRecordRouteRequired(isRecordRouteRequired());
	mesg.getMIME().setRoute(to_.getHostName ());
	sendMessage ( mesg, common.getCallerIp ( ), common.getCallerPort ( ) /*5060*/ );
    mesg.getMIME().setContact("");
	byeSent = true;
	return false;
}

bool SipAnswerLegThread :: handleError ( SIP :: PDU & mesg ) {
	const SIP :: MIMEInfo & mime = mesg.getMIME ( );
	PSYSTEMLOG ( Error, "error response: " << mesg.getStatusCode ( ) << ' ' << mesg.getInfo ( ) << ' ' <<
		mime.getCSeq ( ) );
	int cause;
	conf -> getSIPToH323ErrorResponse ( mesg.getStatusCode ( ), & cause, 0 );
	common.setDisconnectCause ( cause );
	conf -> replaceReleaseCompleteCause ( common, true );
	call -> released ( common.getDisconnectCause ( ) );
	byeSent = true;
	return false;
}

ss :: string SipAnswerLegThread :: parseToTag(const ss :: string& strToTag)
{
	ss :: string :: size_type stBeginTag = strToTag.find("tag=");
	ss :: string strTag;
	if( ss :: string :: npos != stBeginTag )
	{
		strTag = strToTag.substr(stBeginTag + std :: strlen("tag=") + 1,
								strToTag.size() - stBeginTag + std :: strlen("tag=") + 1 );
	}
	return strTag;
}

bool SipAnswerLegThread :: handleAck ( SIP :: PDU & mesg ) {
	if ( strTag_.empty ( ) )
		strTag_ = parseToTag ( mesg.getMIME ( ).getFrom ( ) );
	return true;
}

bool SipAnswerLegThread :: handleCancel ( SIP :: PDU & mesg ) {
	//common.setDisconnectCause ( Q931 :: cvNoRouteToDestination );
    //common.setDisconnectCause ( Q931 :: cvNoCircuitChannelAvailable );
    common.setDisconnectCause ( conf -> getDefaultDisconnectCause() );

	conf -> replaceReleaseCompleteCause ( common, true );
	call -> released ( common.getDisconnectCause ( ) );
	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "OK" );
	sendMessage ( mesg, common.getCallerIp ( ), common.getCallerPort ( ) );
	canceled_ = true;

	PSYSTEMLOG(Info, "SipAnswerLegThread :: handleCancel: canceled_ = true");

	return false;
}

bool SipAnswerLegThread :: handleTrying ( SIP :: PDU & /*mesg*/ ) {
	return true;
}

bool SipAnswerLegThread :: handleOK ( SIP :: PDU & /*mesg*/ ) {
	call -> onHoldOK ( this );
	return sendOnHoldAck ( );
}

void SipAnswerLegThread :: peerAlertedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & /*outCodec*/ ) {
	savedLocalRtpPort = localRtpPort;
	savedInCodec = inCodec;
	sendRinging ( localRtpPort, localAddr, inCodec );
}

void SipAnswerLegThread :: sendRinging ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec ) {

	SIP :: PDU mesg;
	mesg.setStatusCode ( SIP :: PDU :: scInformationRinging );
	mesg.setInfo ( "Ringing" );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( via );
	mime.setFrom ( from_.str ( ) );
	mime.setTo ( to_.str ( ) );
	mime.setCallID ( callId_ );
	mime.setContact ( contact_.bracketShortForm ( ) );
	mime.setCSeq ( inviteCseq );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	if ( inCodec.getCodec ( ) != "unknown" ) {
		SIP :: SessionDescription * sdp = new SIP :: SessionDescription;
		ss :: string lip = intrfIp;
		if ( localAddr != INADDR_ANY )
			lip = static_cast < const char * > ( localAddr.AsString ( ) );
		sdp -> setDefaultConnectAddress ( lip );
		sdp -> setOwnerAddress ( localIp );

		SIP :: MediaDescription media ( SIP :: TransportAddress ( lip, short ( localRtpPort ) ),
			SIP :: MediaDescription :: mtAudio, SIP :: MediaDescription :: ttRtpAvp );
		addFormat ( media, inCodec );
/*		if ( unsigned payload = common.getTelephoneEventsPayloadType ( ) ) {
			SIP :: MediaFormat formt ( payload, "telephone-event" );
			formt.setFMTP ( "0-15" );
			media.addMediaFormat ( formt );
		}*/ // known only in sendOK for now
		sdp -> addMediaDescription ( media );
		mesg.setSDP ( sdp );
	}

	mime.setRecordRouteRequired ( isRecordRouteRequired ( ) );
	if ( isRecordRouteRequired ( ) ) {
		for ( unsigned i = 0; i < vRecordRoute.size ( ); ++ i )
			mime.addRecordRoute ( vRecordRoute [ i ], false );
		mime.addRecordRoute ( localIp );
	}

	sendMessage ( mesg, common.getCallerIp ( ), common.getCallerPort ( ) );
}

void SipAnswerLegThread :: peerConnectedLeg ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec, const CodecInfo & /*outCodec*/ ) {
	savedLocalRtpPort = localRtpPort;
	savedInCodec = inCodec;
	sendOk ( localRtpPort, localAddr, inCodec );
}

void SipAnswerLegThread :: sendOk ( int localRtpPort, const PIPSocket :: Address & localAddr,
	const CodecInfo & inCodec ) {
	PSYSTEMLOG(Info, "SipAnswerLegThread :: sendOk. callid = " << callId_);
	oked = true;
	SIP :: PDU mesg;
	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setInfo ( "OK" );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( via );
	mime.setFrom ( from_.str ( ) );
	mime.setTo ( to_.str ( ) );
	mime.setCallID ( callId_ );
	mime.setContact ( contact_.bracketShortForm ( ) );
	mime.setCSeq ( inviteCseq );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );

	mime.setRecordRouteRequired(isRecordRouteRequired());
	if(isRecordRouteRequired())
	{
		for(unsigned i = 0; i < vRecordRoute.size(); ++i)
		{
		    mime.addRecordRoute(vRecordRoute[i], false);
		}
		mime.addRecordRoute(localIp);
	}
//	mime.addRecordRoute(common.getCallerIp ( ));
//	mime.clearRecordRoute();

	SIP :: SessionDescription * sdp = new SIP :: SessionDescription;
//	sdp -> setDefaultConnectAddress ( localIp );

    ss :: string lip = intrfIp;
    if ( localAddr != INADDR_ANY )
        lip = static_cast < const char * > ( localAddr.AsString ( ) );

    if ( !ccd.getDirectIn ( ) || ccd.curChoice ( ) -> getDirectRTP ( ) )
    {
        sdp -> setDefaultConnectAddress(SIP::TransportAddress(intrfIp));
        sdp -> setOwnerAddress ( localIp );
    }
    else
    {
        sdp -> setDefaultConnectAddress(SIP::TransportAddress(lip));
        sdp -> setOwnerAddress ( lip );
    }

	SIP :: MediaDescription media ( SIP :: TransportAddress ( lip, short ( localRtpPort ) ),
		SIP :: MediaDescription :: mtAudio, SIP :: MediaDescription :: ttRtpAvp );
	addFormat ( media, inCodec );
	if ( unsigned payload = common.getTelephoneEventsPayloadType ( ) ) {
		SIP :: MediaFormat formt ( payload, "telephone-event" );
		formt.setFMTP ( "0-15" );
		media.addMediaFormat ( formt );
	}
	sdp -> addMediaDescription ( media );

	mesg.setSDP ( sdp );
	sendMessage ( mesg, common.getCallerIp ( ), common.getCallerPort ( ) );
}

void SipAnswerLegThread :: peerOnHoldLeg ( int /*level*/ ) {
	sendOnHoldInvite ( );
}

void SipAnswerLegThread :: sendOnHoldInvite ( ) {
	SIP :: PDU mesg ( SIP :: PDU :: mInvite, from_ );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( myVia );
	mime.setFrom ( to_.str ( ) );
	mime.setTo ( from_.str ( ) );
	mime.setCallID ( callId_ );
	mime.setContact ( contact_.bracketShortForm ( ) );
	mime.setCSeq ( "2 INVITE" );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );

	SIP :: SessionDescription * sdp = new SIP :: SessionDescription;
	sdp -> setDefaultConnectAddress ( ss :: string ( "0.0.0.0" ) );
	sdp -> setOwnerAddress ( localIp );
	SIP :: MediaDescription media ( SIP :: TransportAddress ( "0.0.0.0", short ( savedLocalRtpPort ) ),
		SIP :: MediaDescription :: mtAudio, SIP :: MediaDescription :: ttRtpAvp );
	addFormat ( media, savedInCodec );
	if ( unsigned payload = common.getTelephoneEventsPayloadType ( ) ) {
		SIP :: MediaFormat formt ( payload, "telephone-event" );
		formt.setFMTP ( "0-15" );
		media.addMediaFormat ( formt );
		media.setAttribute ( "sendonly" );
	}
	sdp -> addMediaDescription ( media );

	mesg.setSDP ( sdp );
	sendMessage ( mesg, common.getCallerIp ( ), common.getCallerPort ( ) );
}

void SipAnswerLegThread :: peerOnHoldOKLeg ( ) {
	sendOnHoldOK ( );
}

void SipAnswerLegThread :: sendOnHoldOK ( ) {
	SIP :: PDU mesg;
	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setInfo ( "OK" );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( myVia );
	mime.setFrom ( from_.str ( ) );
	mime.setTo ( to_.str ( ) );
	mime.setCallID ( callId_ );
	mime.setContact ( contact_.bracketShortForm ( ) );
	mime.setCSeq ( "2 INVITE" );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );

	SIP :: SessionDescription * sdp = new SIP :: SessionDescription;
	sdp -> setDefaultConnectAddress ( ss :: string ( "0.0.0.0" ) );
	sdp -> setOwnerAddress ( localIp );
	SIP :: MediaDescription media ( SIP :: TransportAddress ( "0.0.0.0", short ( savedLocalRtpPort ) ),
		SIP :: MediaDescription :: mtAudio, SIP :: MediaDescription :: ttRtpAvp );
	addFormat ( media, savedInCodec );
	if ( unsigned payload = common.getTelephoneEventsPayloadType ( ) ) {
		SIP :: MediaFormat formt ( payload, "telephone-event" );
		formt.setFMTP ( "0-15" );
		media.addMediaFormat ( formt );
		media.setAttribute ( "sendonly" );
	}
	sdp -> addMediaDescription ( media );

	mesg.setSDP ( sdp );
	sendMessage ( mesg, common.getCallerIp ( ), common.getCallerPort ( ) );
}

bool SipAnswerLegThread :: sendOnHoldAck ( ) {
	SIP :: PDU mesg ( SIP :: PDU :: mAck, from_ );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( myVia );
	mime.setFrom ( to_.str ( ) );
	mime.setTo ( from_.str ( ) );
	mime.setCallID ( callId_ );
	mime.setContact ( contact_.bracketShortForm ( ) );
	mime.setCSeq ( "2 ACK" );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );

	return sendMessage ( mesg, common.getCallerIp ( ), common.getCallerPort ( ) );
}

void SipAnswerLegThread :: sendBye ( ) {

	SIP :: PDU mesg ( oked ? SIP :: PDU :: mBye : SIP :: PDU :: mCancel, from_ );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( myVia );

	mime.setFrom ( to_.str ( ) );

	ss :: string strContact = strURI_;

	{
		ss :: string :: size_type beginURI = strContact1_.find(':');
		if(beginURI != ss :: string :: npos)
		{
			ss :: string :: size_type endURI = strContact1_.find('>', beginURI+1);
			//if(endURI == ss :: string :: npos)
			{
			//	endURI = strContact1_.find(';', beginURI+1);
			}

			//if(endURI != ss :: string :: npos /*&& strContact1_.size() > 5*/)
			if(endURI != ss :: string :: npos)
			{
				endURI = endURI - (beginURI+1);
			}
			strContact = strContact1_.substr(beginURI+1, endURI);// strContact1_.size() - ( endURI + ( beginURI + 1 ) ) );
			strContact = "sip:" + strContact;
		}
	}

	ss :: string strFrom = from_.str ();
	ss :: string strTo;
	ss :: ostringstream os;
	if(strTag_.empty())
	{
		os << strFrom;
//		PSYSTEMLOG(Info, "SipAnswerLegThread :: sendBye ( ): without Tag = " << os.str() );
	}
	else
	{
		ss :: string :: size_type begTag = strFrom.find(";tag=");
		if(begTag == ss :: string :: npos)
		{
			os << strFrom << ";tag=" << strTag_;
			//PSYSTEMLOG(Info, "SipAnswerLegThread :: sendBye ( ): new Tag = " << os.str() );
		}
		else
		{
			os << strFrom ;
//			PSYSTEMLOG(Info, "SipAnswerLegThread :: sendBye ( ): old Tag = " << os.str() );
		}
	}
	strTo = os.str();
//	mesg.setURI(strURI_);
	mesg.setURI(strContact);

	mime.setFrom ( to_.str ( ) );
	mime.setTo ( strFrom ); //from.str ( ) );
	mime.setCallID ( callId_ );
//	mime.setContact ( contact.bracketShortForm ( ) ); // 2
	mime.setUserAgent ( "PSBC SIP v2.0" );
	mime.setMaxForwards ( 70 );
	mime.setCSeq ( "101 BYE" );

	if(isRecordRouteRequired())
	{
		mime.setRecordRouteRequired(true);
		ss :: string strRoute;
		for(unsigned i = 0; i < vRecordRoute.size(); ++i)
		{
		    //strRoute = "<sip:";
		    strRoute = vRecordRoute[i];
		    //strRoute += ";lr>";
			if(i+1 < vRecordRoute.size())
			{
				strRoute += ", ";
			}
		}
		mime.setRoute(strRoute, false);
//	mime.setRoute(to.getHostName ());
	}

	sendMessage ( mesg, common.getCallerIp ( ), /*5060*/ common.getCallerPort ( ) );
}

void SipAnswerLegThread :: sendError ( ) {
	SIP :: PDU mesg ( SIP :: PDU :: mNum, from_ );

	int cause = common.getDisconnectCause ( );
	int statusCode = 0;
	ss :: string errorText;
	if ( cause )
		conf -> getH323ToSIPErrorResponse ( cause, & statusCode, & errorText );
	if(canceled_)
	{
		mesg.setStatusCode ( SIP :: PDU :: scFaiureRequestTerminated );
		mesg.setInfo ( "Request Terminated" );
	}
	else
	{
		if(0 == statusCode)
		{
			mesg.setStatusCode ( SIP :: PDU :: scFaiureServiceUnavailable/*scFailureForbidden*/ );
			mesg.setInfo ( "Service Unavailable." );
		}
		else
		{
			mesg.setStatusCode ( statusCode );
			mesg.setInfo ( errorText );
		}
	}
	PSYSTEMLOG(Info, "SipAnswerLegThread :: sendError: statusCode = " << statusCode << "; errorText = " << errorText );

	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( via );
	mime.setFrom ( from_.str ( ) );
	mime.setTo ( to_.str ( ) );
	mime.setCallID ( callId_ );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	mime.setMaxForwards ( 70 );
	mime.setCSeq ( inviteCseq );
	sendMessage ( mesg, common.getCallerIp ( ), /*5060*/ common.getCallerPort ( ) );
}

bool SipAnswerLegThread :: isRecordRouteRequired() const
{
	return ccd.getSigOptions ( ).isRecordRouteRequired ( ); //fix: ne obnovlyaetsya pri huntinge
}

void SipAnswerLegThread :: sendTrying ( SIP :: PDU & mesg ) {
	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( SIP :: PDU :: scInformationTrying );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "Trying" );
	clearContent ( mesg );
	mesg.getMIME ( ).setContact ( "" );
	sendMessage ( mesg, common.getCallerIp ( ), common.getCallerPort ( ) );
}

bool SipAnswerLegThread :: handleInvite ( SIP :: PDU & mesg ) {
	if ( mesg.hasSDP ( ) && mesg.getSDP ( ).getDefaultConnectAddress ( ).getAddr ( ) == "0.0.0.0" ) {
		call -> onHold ( this, 7 );
		return true;
	}
	PSYSTEMLOG(Info, "SipAnswerLegThread :: handleInvite. Invite was skipped.");
	return true;
}

void SipAnswerLegThread :: putMessage ( const PeerMessage & m ) {
	AutoMutex am ( receiverMut );
	peerQueue.push ( m );
	if ( receiver )
		receiver -> wake ( );
}

bool SipAnswerLegThread :: handleOptions( SIP :: PDU & mesg1 )
{
	PSYSTEMLOG(Info, "SipAnswerLegThread :: handleOptions.");
	SIP :: PDU mesg;
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( via );
	mime.setFrom ( from_.str ( ) );

	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setInfo ( "OK" );

	mime.setTo ( to_.str ( ) );
	mime.setCallID ( callId_ );
	mime.setContact ( from_.bracketShortForm ( ) );
	mime.setCSeq ( mesg1.getMIME().getCSeq() );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	mime.setAcceptEncoding("identity");
	mime.setAllow( "INVITE, ACK, CANCEL, INFO, OPTIONS, BYE" );
	mime.setSupported("");
	return sendMessage ( mesg, common.getCallerIp ( ), common.getCallerPort ( ) );
}

bool SipAnswerLegThread :: handleInfo( SIP :: PDU & mesg1 )
{
	PSYSTEMLOG(Info, "SipAnswerLegThread :: handleInfo.");
	SIP :: PDU mesg;
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( via );
	mime.setFrom ( from_.str ( ) );

	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setInfo ( "OK" );

	mime.setTo ( to_.str ( ) );
	mime.setCallID ( callId_ );
	mime.setContact ( from_.bracketShortForm ( ) );
	mime.setCSeq ( mesg1.getMIME().getCSeq() );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );
//	insertPAssertId(mesg);
/*
	mime.setRecordRouteRequired(isRecordRouteRequired());
	if(isRecordRouteRequired())
	{
		mesg.setRecordRoute(mesg1.getRecordRoute ( ));
	}
*/
	return sendMessage ( mesg, common.getCallerIp ( ), common.getCallerPort ( ) );
}



