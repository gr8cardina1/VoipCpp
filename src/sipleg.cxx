#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "pointer.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "codecinfo.hpp"
#include "signallingoptions.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoicedetails.hpp"
#include "sourcedata.hpp"
#include "aftertask.hpp"
#include <ptlib.h>
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include <ptlib/sockets.h>
#include "q931.hpp"
#include "callcontrol.hpp"
#include "legthread.hpp"
#include <queue>
#include "originatelegthread.hpp"
#include "ixcudpsocket.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include "sip.hpp"
#include "SIPAuthenticateValidator.hpp"
#include "sipleg.hpp"
#include "sipcommon.hpp"
#include "condvar.hpp"
#include "automutex.hpp"
#include "threadmessagequeue.hpp"
#include "sipsignallingthread.hpp"
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <ptlib/svcproc.h>
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <boost/range.hpp>
#include "slprint.hpp"
#include <cstring>

static ss :: string getLocalAddress  ( ) {
	PIPSocket :: Address addr;
        PIPSocket :: GetHostAddress ( addr );
	return static_cast < const char * > ( addr.AsString ( ) );
}

SipLeg :: SipLeg ( OriginateLegThread * t, CommonCallDetails & c, const RecodeInfoVector & inCodecs,
	PMutex * wm, unsigned ref, bool isRecordRouteRequired ) :
	Leg ( t, c ), wakeMut ( wm ), tag( toHex ( c.getConfId ( ) ).substr ( 0, 15 ) ),
	receiver ( 0 ), callId( toHex ( c.getConfId ( ) ).substr ( 0, 15 ) ),
	oked ( false ), byeSent ( false ), inRecodes ( inCodecs ),
	isRecordRouteRequired_(isRecordRouteRequired),
    isAccSend_(false),
    accountCard_(""),
    directRTP_(c.getDirectIn ( ) || c.curChoice ( ) -> getDirectRTP ( ))
{
	PSYSTEMLOG(Info, "SipLeg :: SipLeg: " << callId << "; toHex ( c.getConfId ( ) ) = " <<
		toHex ( c.getConfId ( ) ) << "; c.getConfId ( ) = " << c.getConfId ( ) );

	ss :: ostringstream os;
	os << "z9hG4bK." << std :: hex << ref;
	branch = os.str ( );
}

bool SipLeg :: tryChoice ( ) {
	if ( ! sipThread )
		return false; // race condition at start
	common.sigOptions ( ).setOut ( common.curChoice ( ) -> getSigOptions ( ) );
    if ( common.getSigOptions ( ). isAccSend() )
    {
        bool isCard = conf ->isRegisteredCardAddr(common.curChoice ( ) -> getIp ( ),
                                                  WORD ( common.curChoice ( ) -> getPort ( ) ),
                                                  accountCard_);
        if(isCard)
        {
            to_.setUserName(accountCard_);
            isAccSend_ = true;
            PSYSTEMLOG(Info, "SipLeg :: tryChoice: to: " << to_.str() << "; accountCard = " << accountCard_ );
        }
    }

	int addr = common.curChoice ( ) -> getInterfaces ( ) [ 0 ];
	if ( addr == INADDR_ANY )
		localIp = getLocalAddress ( );
	else
		localIp = static_cast < const char * > ( PIPSocket :: Address ( addr ).AsString ( ) );
	to_.setHostName ( common.curChoice ( ) -> getIp ( ) );
	to_.setPort ( short ( common.curChoice ( ) -> getPort ( ) ) );
    strSentDigits_ = common.getSentDigits ( );
    if(false == isAccSend_)
        to_.setUserName ( common.getSentDigits ( ) );

	if ( common.getCallingDigits ( ).empty ( ) )
		from_.setUserName ( common.getCallingDigitsIn ( ) );
	else if ( common.getCallingDigits ( ) == "-" )
		from_.setUserName ( "" );
	else
		from_.setUserName ( common.getCallingDigits ( ) );
	from_.setHostName ( localIp );
	from_.setPort ( 5060 );
	from_.setSipParam ( "tag", tag );
	via = "SIP/2.0/UDP " + localIp + ";branch=" + branch;
	thread -> calledSocketConnected ( PIPSocket :: Address ( addr ) );
	return true;
}

bool SipLeg :: initChoice ( ) {
	return sendInvite ( );
}

bool SipLeg :: ended ( ) const {
	return false;
}

void SipLeg :: wakeUp ( ) {
	if ( receiver )
		receiver -> wake ( );
}

static void addFormats ( SIP :: MediaDescription & media, const RecodeInfoVector & codecs ) {
	using boost :: lambda :: _1;
	ss :: for_each ( codecs, boost :: lambda :: bind ( addFormat, boost :: lambda :: var ( media ),
		& _1 ->* & RecodeInfo :: codec ) );
}

bool SipLeg :: sendInvite ( const ss :: string & authLine, int authType ) {
	PSYSTEMLOG(Info, "SipLeg :: sendInvite: callId = " << callId);
	if ( authLine == "" ) {	 // first INVITE
		receiver = sipThread -> addReceiver ( callId );
		if ( ! receiver )
		{
			PSYSTEMLOG(Info, "SipLeg :: sendInvite: Wrongs receiver.");
			return false;
		}
	} // else - second INVITE with Authorization
	SIP :: PDU mesg ( SIP :: PDU :: mInvite, to_ );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	ss :: string gkIp = common.curChoice ( ) -> getIp ( );
	ss :: string gkLogin, gkPswd, gkCallId;
	int gkPort;
	if ( conf -> getGkInfo ( gkIp, gkLogin, gkPswd, gkPort ) ) {
		from_.setUserName ( gkLogin );
		from_.setHostName ( gkIp );
		from_.setPort ( short ( gkPort ) );
	}
	switch ( authType ) {
		case SIP :: PDU :: scFailureUnAuthorised :
			mime.setAuthorization ( authLine );
			break;
		case SIP :: PDU :: scFailureProxyAuthRequired :
			mime.setProxyAuthorization ( authLine );
			break;
		default :
			PSYSTEMLOG ( Error, "UnAuthorised: Unsupported statusCode " << authType );
	}
	mime.setVia ( via );

	if(common.isAnonymous())
	{
        ss :: string myFrom = createAnonimousFromHeader(from_.str ( ), "\"Anonymous\" <sip:Anonymous@Anonymous.invalid>");
		mime.setFrom ( myFrom );
		mime.addPrivacyHeader();
        SIP :: URL contact = myFrom;
        contact.setHostName ( getLocalAddress ( ) );
        contact.setPort ( 5060 );
        mime.setContact ( contact.bracketShortForm ( ) );
	}
	else
	{
		mime.setFrom ( from_.str ( ) );
        mime.setContact ( from_.bracketShortForm ( ) );
	}

	mime.setTo ( to_.str ( ) );
	mime.setCallID ( callId );
	if ( authLine != "" )
		mime.setCSeq ( "2 INVITE" );
	else
		mime.setCSeq ( "1 INVITE" );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	mime.setExpires ( "180" );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );

	createPAssertId(from_.str( ));
	insertPAssertId(mesg);

	std :: auto_ptr < SIP :: SessionDescription > sdp ( new SIP :: SessionDescription );

    int port = thread -> getLocalPort ( );
    PIPSocket :: Address lip = thread -> getLocalIp ( );
    ss :: string rtpIP = lip == INADDR_ANY ? localIp : static_cast < const char * > ( lip.AsString ( ) );

    if(! directRTP_)
	{
        sdp -> setDefaultConnectAddress ( localIp );
	    sdp -> setOwnerAddress ( localIp );
    }
    else
    {
        sdp -> setOwnerAddress ( SIP :: TransportAddress ( rtpIP, short ( port ) ) );
        sdp -> setDefaultConnectAddress ( SIP :: TransportAddress ( rtpIP, short ( port ) ) );
    }

    PSYSTEMLOG(Info, "SipLeg :: sendInvite direct " << directRTP_ << "; rtpIP " << rtpIP << "; localIp " << localIp 
               << "; lip "  << lip   );

    mime.setRecordRouteRequired(isRecordRouteRequired());
	if(isRecordRouteRequired())
	{
	    mime.clearRecordRoute();
		mime.addRecordRoute(localIp);
	}

	SIP :: MediaDescription media ( SIP :: TransportAddress ( rtpIP, short ( port ) ), SIP :: MediaDescription :: mtAudio,
		SIP :: MediaDescription :: ttRtpAvp );
 	addFormats ( media, inRecodes );
//	PSYSTEMLOG(Info, "SipLeg :: sendInvite: media = \n" << media );


//	PSYSTEMLOG(Info, "SipLeg :: sendInvite: payloadType = " << common.getTelephoneEventsPayloadType () << "; In = " <<
//		common.getSigOptions ( ).isForceInsertingDTMFintoInviteTerm ( ) << "; Out = " << common.getSigOptions ( ).isForceInsertingDTMFintoInviteOrig ( ));
	if ( ! common.getTelephoneEventsPayloadType ( ) && common.getSigOptions ( ).isForceInsertingDTMFintoInvite ( ) )
		common.setTelephoneEventsPayloadType ( allocTelephoneEventsPayload ( sdp -> getMediaDescriptions ( ) ) );
	if ( unsigned payload = common.getTelephoneEventsPayloadType ( ) ) {
		media.setTelephoneEventsPayloadType ( payload );
		SIP :: MediaFormat formt ( payload, "telephone-event" );
		formt.setFMTP ( "0-15" );
		media.addMediaFormat ( formt );
		thread -> setTelephoneEventsPayloadType ( payload );
	}
	sdp -> addMediaDescription ( media );
	mesg.setSDP ( sdp.release ( ) );

	return sendMessage ( mesg, to_.getHostName ( ), to_.getPort ( ) );
}

bool SipLeg :: peerOnHold ( int /*level*/ ) {
	SIP :: PDU mesg ( SIP :: PDU :: mInvite, to_ );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	ss :: string gkIp = common.curChoice ( ) -> getIp ( );
	mime.setVia ( via );

	mime.setFrom ( from_.str ( ) );
        mime.setContact ( from_.bracketShortForm ( ) );

	mime.setTo ( to_.str ( ) );
	mime.setCallID ( callId );
	mime.setCSeq ( "2 INVITE" );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	mime.setExpires ( "180" );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );

	std :: auto_ptr < SIP :: SessionDescription > sdp ( new SIP :: SessionDescription );

	sdp -> setDefaultConnectAddress ( ss :: string ( "0.0.0.0" ) );
	sdp -> setOwnerAddress ( localIp );
	int port = thread -> getLocalPort ( );

	SIP :: MediaDescription media ( SIP :: TransportAddress ( "0.0.0.0", short ( port ) ), SIP :: MediaDescription :: mtAudio,
		SIP :: MediaDescription :: ttRtpAvp );
 	addFormats ( media, inRecodes );

	if ( ! common.getTelephoneEventsPayloadType ( ) && common.getSigOptions ( ).isForceInsertingDTMFintoInvite ( ) )
		common.setTelephoneEventsPayloadType ( allocTelephoneEventsPayload ( sdp -> getMediaDescriptions ( ) ) );
	if ( unsigned payload = common.getTelephoneEventsPayloadType ( ) ) {
		media.setTelephoneEventsPayloadType ( payload );
		SIP :: MediaFormat formt ( payload, "telephone-event" );
		formt.setFMTP ( "0-15" );
		media.addMediaFormat ( formt );
		media.setAttribute ( "sendonly" );
	}
	sdp -> addMediaDescription ( media );
	mesg.setSDP ( sdp.release ( ) );

	return sendMessage ( mesg, to_.getHostName ( ), to_.getPort ( ) );
}

bool SipLeg :: peerOnHoldOK ( ) {
	SIP :: PDU mesg;
	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setInfo ( "OK" );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( via );
	mime.setFrom ( to_.str ( ) );
	mime.setTo ( from_.str ( ) );
	mime.setCallID ( callId );
	mime.setContact ( from_.bracketShortForm ( ) );
	mime.setCSeq ( "2 INVITE" );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );

	std :: auto_ptr < SIP :: SessionDescription > sdp ( new SIP :: SessionDescription );

	sdp -> setDefaultConnectAddress ( localIp );
	sdp -> setOwnerAddress ( localIp );
	int port = thread -> getLocalPort ( );

	PIPSocket :: Address lip = thread -> getLocalIp ( );
	ss :: string rtpIP = lip == INADDR_ANY ? localIp : static_cast < const char * > ( lip.AsString ( ) );

	SIP :: MediaDescription media ( SIP :: TransportAddress ( rtpIP, short ( port ) ), SIP :: MediaDescription :: mtAudio,
		SIP :: MediaDescription :: ttRtpAvp );
 	addFormats ( media, inRecodes );

	if ( ! common.getTelephoneEventsPayloadType ( ) && common.getSigOptions ( ).isForceInsertingDTMFintoInvite ( ) )
		common.setTelephoneEventsPayloadType ( allocTelephoneEventsPayload ( sdp -> getMediaDescriptions ( ) ) );
	if ( unsigned payload = common.getTelephoneEventsPayloadType ( ) ) {
		media.setTelephoneEventsPayloadType ( payload );
		SIP :: MediaFormat formt ( payload, "telephone-event" );
		formt.setFMTP ( "0-15" );
		media.addMediaFormat ( formt );
		thread -> setTelephoneEventsPayloadType ( payload );
	}
	sdp -> addMediaDescription ( media );
	mesg.setSDP ( sdp.release ( ) );

	return sendMessage ( mesg, to_.getHostName ( ), to_.getPort ( ) );
}

bool SipLeg :: sendMessage ( SIP :: PDU & mesg, const ss :: string & addr, int port ) {
	bool ret = sipThread -> sendMessage ( mesg, PIPSocket :: Address ( addr.c_str ( ) ), port );
//	PSYSTEMLOG(Info, "SipLeg :: sendMessage: callId = " << ret);
	return ret;
}

ss :: string SipLeg :: createToHeader(const ss :: string& strAddr)
{
	ss :: ostringstream os;
	if(strToTag_.empty())
	{
		os << strAddr;
	}
	else
	{
		os << strAddr << ";tag=" << strToTag_;
	}
	return os.str();
}

ss :: string SipLeg :: createAnonimousFromHeader(const ss :: string& strFrom, const ss :: string& strAddr)
{
	if(common.isAnonymous())
	{
		if(false == strFrom.empty())
		{
			ss :: string :: size_type stBeginTag = strFrom.find("tag=");
			if( ss :: string :: npos != stBeginTag )
			{
				ss :: string strTag = strFrom.substr(stBeginTag + std :: strlen("tag=") + 1, strFrom.size() - stBeginTag + std :: strlen("tag=") + 1 );
				ss :: ostringstream os;
				if(strTag.empty())
				{	//os << "\"Anonymous\" <sip:Anonymous@Anonymous.invalid>";
					os << strAddr;
				}
				else
				{//	os << "\"Anonymous\" <sip:Anonymous@Anonymous.invalid>;tag=" << strTag;
					os << strAddr << ";tag=" << strTag;
				}
				return os.str();
			}
		}
	}

	return from_.str ( );
}

void SipLeg :: closeChoice ( ) {
//	PSYSTEMLOG(Info, "SipLeg :: closeChoice: isRecordRouteRequired = " << isRecordRouteRequired());
	if ( receiver ) {
		AutoMutex am ( wakeMut );
		sipThread -> removeReceiver ( callId );
		receiver = 0;
	}
	if ( byeSent )
		return;
	byeSent = true;
	SIP :: PDU mesg ( oked ? SIP :: PDU :: mBye : SIP :: PDU :: mCancel, to_ );
//	mesg.setContact(strContactRoute_);
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( via );

	mime.setFrom(createAnonimousFromHeader(from_.str ( ), "\"Anonymous\" <sip:Anonymous@Anonymous.invalid>"));
	if(isRecordRouteRequired())
	{
		if(oked)
		{ // BYE
			mime.setTo(to_.str ( ));
		}
		else
		{ // CANCEL
			mime.setTo ( to_.bracketShortForm ( ));
		}
	}
	else
	{
		if(oked)
		{ // BYE
			mime.setTo(to_.str ( ));
		}
		else
		{ // CANCEL
			mime.setTo ( to_.bracketShortForm ( ));
		}
	}

	mime.setCallID ( callId );
	mime.setCSeq ( oked ? "101 BYE" : "1 CANCEL" );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	insertPAssertId(mesg);
	mime.setRecordRouteRequired(isRecordRouteRequired());

	if(false == oked) // CANCEL
	{
		mesg.setURI(to_);
		if(isRecordRouteRequired())
		{
		    mime.clearRecordRoute();
			mime.addRecordRoute(to_.getHostName ());
		}
	}
	else
	{
        if(false == strContactFromOK_.empty())
            mesg.setURI(strContactFromOK_);

		mime.setRoute(to_.getHostName ());
	}

	sendMessage ( mesg, to_.getHostName ( ), to_.getPort ( ) );
}

bool SipLeg :: receiveMessage ( SIP :: PDU & mesg ) {
	return receiver -> get ( mesg );
}

bool SipLeg :: iteration ( ) {
	SIP :: PDU mesg;
	if ( ! receiveMessage ( mesg ) )
		return true;
	return handleMessage ( mesg );
/*
	SIP :: PDU mesg;
	bool retVal = false;
	if ( ! receiveMessage ( mesg ) )
		retVal = true;
	if(true == retVal)
		retVal = handleMessage ( mesg );
	PSYSTEMLOG(Info, "SipLeg :: iteration " << retVal);
	return retVal;
*/
}

static bool isPreviousHunt ( const SIP :: URL & u1, const SIP :: URL & u2 ) {
	if ( u1.getHostName ( ) != u2.getHostName ( ) || u1.getUserName ( ) != u2.getUserName ( ) ||
		u1.getPort ( ) != u2.getPort ( ) ) {
		PSYSTEMLOG ( Error, "From previous hunt" );
		return true;
	}
	return false;
}

bool SipLeg :: isPreviousHuntTo ( const SIP :: MIMEInfo & mime ) {
	return isPreviousHunt ( mime.getTo ( ), to_ );
}

bool SipLeg :: isPreviousHuntFrom ( const SIP :: MIMEInfo & mime ) {
	return isPreviousHunt ( mime.getFrom ( ), to_ );
}

bool SipLeg :: handleTrying ( SIP :: PDU & mesg ) {
	if ( isPreviousHuntTo ( mesg.getMIME ( ) ) )
		return true;
	to_ = mesg.getMIME ( ).getTo ( );

//	if(strContactRoute_.empty())
	{
		strContactRoute_ = parseContact(mesg.getMIME().getContact());
		strToTag_ = parseToTag(mesg.getMIME().getTo());
		PSYSTEMLOG(Info, "SipLeg :: handleTrying: Contact = " << strContactRoute_ << "; tagTo = " << strToTag_);
	}

	return true;
}

ss :: string SipLeg :: parseContact(const ss :: string& strContact)
{
	ss :: string strNewContact = strContact;
/*
	for(int i = 0; i < strContact.size(); i++)
	{

	}
*/
	return strNewContact;
}

ss :: string SipLeg :: parseToTag(const ss :: string& strToTag)
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


bool SipLeg :: handleAnswerSDP ( SIP :: PDU & mesg ) {
	PSYSTEMLOG(Info, "SipLeg :: handleAnswerSDP" );
	if ( ! mesg.hasSDP ( ) )
		return false;
	SIP :: SessionDescription & sdp = mesg.getSDP ( );
	if ( SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) ) {
		if ( unsigned payload = media -> getTelephoneEventsPayloadType ( ) )
			common.setTelephoneEventsPayloadType ( payload );
		if ( unsigned payload = common.getTelephoneEventsPayloadType ( ) ) {
			//PSYSTEMLOG(Info, "SipLeg :: handleSDP1. set payload " << payload);
			thread -> setTelephoneEventsPayloadType ( payload );
		}
		const SIP :: MediaFormatVector & formats = media -> getMediaFormats ( );
		for ( SIP :: MediaFormatVector :: const_iterator i = formats.begin ( ); i != formats.end ( ); ++ i ) {
			CodecInfo codec = getCodec ( * i ), changedCodec = codec;
			typedef RecodeInfoVector :: index < Codec > :: type ByCodec;
			ByCodec & inByCodec = inRecodes.get < Codec > ( );

            //PSYSTEMLOG(Info, "handleAnswerSDP : " << codec.getCodec());
///     codec.getCodec() - from terminator
///     i -> codec.getCodec() - from originator
            if( CompareCodecs :: is729 ( codec.getCodec() ) )
            {
                for(ByCodec :: const_iterator i = inByCodec.begin(); i != inByCodec.end ( ); ++ i)
                {
                    //PSYSTEMLOG(Info, "handleAnswerSDP incodes : " << i -> codec.getCodec());
                    if(CompareCodecs :: is729 (i -> codec.getCodec()))
                    {
                        if( i -> codec.getCodec() == codec.getCodec() )
                        {
                            codec = i -> backCodec;
                            PIPSocket :: Address addr;
                            WORD port;
                            media -> getTransportAddress ( ).getIpAndPort ( addr, port );
                            thread -> setCodec ( codec );

                            media -> setTransportAddress ( SIP :: TransportAddress ( getLocalAddress ( ), port ) );

                            thread -> setSendAddress ( addr, port, codec, codec, changedCodec, changedCodec );
                            //PSYSTEMLOG(Info, "handleAnswerSDP : 1");
                            return true;
                        }
                        else
                        {
                            if (CompareCodecs :: isAnnexB (i -> codec.getCodec()))
                            {
                                //PSYSTEMLOG(Info, "handleAnswerSDP : 20: " << codec.getCodec());
                                //codec = i -> backCodec;
                                PIPSocket :: Address addr;
                                WORD port;
                                media -> getTransportAddress ( ).getIpAndPort ( addr, port );
                                thread -> setCodec ( codec );

                                media -> setTransportAddress ( SIP :: TransportAddress ( getLocalAddress ( ), port ) );

                                thread -> setSendAddress ( addr, port, codec, codec, changedCodec, changedCodec );
                                //PSYSTEMLOG(Info, "handleAnswerSDP : 2: " << codec.getCodec());
                                return true;

                            }
                            else
                            {
                                if(CompareCodecs :: isAnnexB (codec.getCodec()))
                                {
                                  //  PSYSTEMLOG(Info, "handleAnswerSDP : 3");
                                    return false;
                                }
                                else
                                {
                                    codec = i -> backCodec;
                                    PIPSocket :: Address addr;
                                    WORD port;
                                    media -> getTransportAddress ( ).getIpAndPort ( addr, port );
                                    thread -> setCodec ( codec );

                                    thread -> setSendAddress ( addr, port, codec, codec, changedCodec, changedCodec );
                          //          PSYSTEMLOG(Info, "handleAnswerSDP : 4");
                                    return true;

                                }
                            }
                        }
                    }
                    else
                    {
                        //PSYSTEMLOG(Info, "handleAnswerSDP : 5");
                        continue;
                    }
                }
            }
            else
            {
                ByCodec :: const_iterator i = inByCodec.find ( codec );
                if ( i == inByCodec.end ( ) ) {
                    PSYSTEMLOG ( Error, "inByCodec not found: " << codec );
                    ss :: for_each ( inByCodec, SLPrint ( "recode: " ) );
                    continue;
                } else
                    codec = i -> backCodec;
                PIPSocket :: Address addr;
                WORD port;
                media -> getTransportAddress ( ).getIpAndPort ( addr, port );
                thread -> setCodec ( codec );

                thread -> setSendAddress ( addr, port, codec, codec, changedCodec, changedCodec );
                //PSYSTEMLOG(Info, "handleAnswerSDP : 6");
                return true;
            }
		}
	}
    //PSYSTEMLOG(Info, "handleAnswerSDP : 7");
	return false;
}

bool SipLeg :: handleSessionProgress ( SIP :: PDU & mesg ) {
	if ( isPreviousHuntTo ( mesg.getMIME ( ) ) )
		return true;
	to_ = mesg.getMIME ( ).getTo ( );
//	if ( handleAnswerSDP ( mesg ) )
	handleAnswerSDP ( mesg );

	thread -> alerted ( );
	{
		strContactRoute_ = mesg.getMIME().getContact();
		strToTag_ = parseToTag(mesg.getMIME().getTo());
        PSYSTEMLOG(Info, "SipLeg :: handleSessionProgress: Contact = " << strContactRoute_ << "; tagTo = " << strToTag_);

        if ( isAccSend_ )
        {
            mesg.getMIME().setTo( accountCard_ );
        }


		if(mesg.hasSDP())
		{
			bool isPresentTelephoneEvent = false;
			SIP :: SessionDescription& sdp = mesg.getSDP();
			if ( SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) )
			{
				const SIP :: MediaFormatVector &mediaFormats = media -> getMediaFormats ( );
				for ( SIP :: MediaFormatVector :: const_iterator i = mediaFormats.begin ( ); i != mediaFormats.end ( ); ++ i )
				{
					const SIP :: MediaFormat& format = *i;
					if( format.getEncodingName() == "telephone-event" )
					{
						isPresentTelephoneEvent = true;
					}
				}
				if( false == isPresentTelephoneEvent )
				{
					SIP :: MediaFormat formt( 101, "telephone-event" );
					formt.setFMTP("a=fmtp:101 0-15");
					media -> addMediaFormat ( formt ); // !!!!!!!!!!!!!!!
				}
			}
		}

	}

	return true;
}

bool SipLeg :: handleRinging ( SIP :: PDU & mesg ) {
	if ( isPreviousHuntTo ( mesg.getMIME ( ) ) )
		return true;
	to_ = mesg.getMIME ( ).getTo ( );
//	if ( handleAnswerSDP ( mesg ) )
	handleAnswerSDP ( mesg );

	thread -> alerted ( );
	{
		strContactRoute_ = mesg.getMIME().getContact();
		strToTag_ = parseToTag(mesg.getMIME().getTo());
        PSYSTEMLOG(Info, "SipLeg :: handleRinging: Contact = " << strContactRoute_ << "; tagTo = " << strToTag_);

        if ( isAccSend_ )
        {
            mesg.getMIME().setTo( accountCard_ );
            //PSYSTEMLOG(Info, "SipLeg :: handleRinging. isAccSend = true. to = " << call -> to);
        }

/*
		if(mesg.hasSDP())
		{
			bool isPresentTelephoneEvent = false;
			SIP :: SessionDescription& sdp = mesg.getSDP();
			if ( SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) )
			{
				const SIP :: MediaFormatVector &mediaFormats = media -> getMediaFormats ( );
				for ( SIP :: MediaFormatVector :: const_iterator i = mediaFormats.begin ( ); i != mediaFormats.end ( ); ++ i )
				{
					const SIP :: MediaFormat& format = *i;
					if( format.getEncodingName() == "telephone-event" )
					{
						isPresentTelephoneEvent = true;
					}
				}
				if( false == isPresentTelephoneEvent )
				{
					SIP :: MediaFormat formt( 101, "telephone-event" );
					formt.setFMTP("a=fmtp:101 0-15");
					media -> addMediaFormat ( formt ); // !!!!!!!!!!!!!!!
				}
			}
		}
*/

/*
		if(mesg.hasSDP())
		{
			bool isPresentTelephoneEvent = false;
			SIP :: SessionDescription& sdp = mesg.getSDP();
			if ( SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) )
			{
				const SIP :: MediaFormatVector &mediaFormats = media -> getMediaFormats ( );
				for ( SIP :: MediaFormatVector :: const_iterator i = mediaFormats.begin ( ); i != mediaFormats.end ( ); ++ i )
				{
					const SIP :: MediaFormat& format = *i;
					if( format.getEncodingName() == "telephone-event" )
					{
						isPresentTelephoneEvent = true;
					}
				}
				if( false == isPresentTelephoneEvent )
				{
					SIP :: MediaFormat formt( 101, "telephone-event" );
					formt.setFMTP("a=fmtp:101 0-15");
					media -> addMediaFormat ( formt ); // !!!!!!!!!!!!!!!
				}
			}
		}
*/
	}

	return true;
}

bool SipLeg :: sendAck ( const ss :: string & cseq, const StringVector* pRecordRoute, const ss :: string * contact ) {
	SIP :: PDU mesg ( SIP :: PDU :: mAck, to_ );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( via );
	mime.setFrom ( createAnonimousFromHeader(from_.str ( ), "\"Anonymous\" <sip:Anonymous@Anonymous.invalid>"));

	mime.setTo ( to_.str ( ) );
	mime.setCallID ( callId );

    SIP :: URL contact1 = mime.getFrom();
    contact1.setHostName ( getLocalAddress ( ) );
    contact1.setPort ( 5060 );
    mime.setContact ( contact1.bracketShortForm ( ) );

    mime.setCSeq ( cseq );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	insertPAssertId(mesg);

	mime.setRecordRouteRequired(isRecordRouteRequired());
	if(pRecordRoute && false == pRecordRoute->empty())
	{
		StringVector recordRoute(*pRecordRoute);
//		std :: reverse_copy (pRecordRoute->begin(), pRecordRoute->end(), recordRoute.begin());
//		StringVector :: reverse_iterator it = pRecordRoute->rbegin();

//		for(; it != pRecordRoute->rend(); ++ it)
//		{
//			recordRoute.push_back( *it );
//		}
		reverse(recordRoute.begin(), recordRoute.end());
		recordRoute.pop_back();
		if(false == recordRoute.empty())
		{
//			for(unsigned i = 0; i < recordRoute.size(); ++ i)
//				PSYSTEMLOG(Info, "SipLeg :: sendAck: " << recordRoute[i]);
//			PSYSTEMLOG(Info, "SipLeg :: sendAck. size " << recordRoute.size());
			mime.setRoute(recordRoute);
		}
	}
	else
	{
		mime.setRoute(to_.getHostName ( ));
	}

	if(contact && false == contact->empty())
	{
		mesg.setURI(*contact);
	}

	return sendMessage ( mesg, to_.getHostName ( ), to_.getPort ( ) );
}

bool SipLeg :: handleCancelOK ( SIP :: PDU & ) {
	return false;
}

bool SipLeg :: handleByeOK ( SIP :: PDU & ) {
	return false;
}

bool SipLeg :: handleOK ( SIP :: PDU & mesg ) {
	const SIP :: MIMEInfo & mime = mesg.getMIME ( );
	if ( isPreviousHuntTo ( mime ) )
		return true;
	const ss :: string & cseq = mime.getCSeq ( );
	if ( cseq.find ( "CANCEL" ) != ss :: string :: npos )
		return handleCancelOK ( mesg );
	if ( cseq.find ( "BYE" ) != ss :: string :: npos )
		return handleByeOK ( mesg );
	PSYSTEMLOG(Info, "SipLeg :: handleOK");
	to_ = mime.getTo ( );
	oked = true;
	if ( ! handleAnswerSDP ( mesg ) )
		return false;
	thread -> connected ( );

    strContactFromOK_ = mime.getContact();

/*
	PSYSTEMLOG(Info, "SipLeg :: handleOK");

	if(mesg.hasSDP())
	{
		PSYSTEMLOG(Info, "SipLeg :: handleOK. SDP is present.");
		bool isPresentTelephoneEvent = false;
		SIP :: SessionDescription& sdp = mesg.getSDP();
		if ( SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) )
		{
			PSYSTEMLOG(Info, "SipLeg :: handleOK. media present");
			const SIP :: MediaFormatVector &mediaFormats = media -> getMediaFormats ( );
			for ( SIP :: MediaFormatVector :: const_iterator i = mediaFormats.begin ( ); i != mediaFormats.end ( ); ++ i )
			{
				const SIP :: MediaFormat& format = *i;
				if( format.getEncodingName() == "telephone-event" )
				{
					isPresentTelephoneEvent = true;
				}
			}
			if( false == isPresentTelephoneEvent )
			{
				SIP :: MediaFormat formt( 101, "telephone-event" );
				formt.setFMTP("a=fmtp:101 0-15");
				media->addMediaFormat ( formt ); // !!!!!!!!!!!!!!!
			}
		}
	}
*/
	StringVector v (mime.getRecordRoute ());
	ss :: string s (mime.getContact());
	return sendAck ( changeCSeq ( mime.getCSeq ( ), "ACK" ), &v, &s );
}

bool SipLeg :: handleError ( SIP :: PDU & mesg ) {
	const SIP :: MIMEInfo & mime = mesg.getMIME ( );
	if ( isPreviousHuntTo ( mime ) )
		return true;

	PSYSTEMLOG ( Info, "SipLeg :: handleError: " << mesg.getStatusCode ( ) );
	int response;
	conf -> getSIPToH323ErrorResponse ( mesg.getStatusCode ( ), & response, 0 );
	thread -> released ( Q931 :: CauseValues ( response ) );
	byeSent = true;

	if ( mesg.getStatusCode ( ) != SIP :: PDU :: scFaiureTransactionDoesNotExist ) {
		StringVector v (mime.getRecordRoute ());
		ss :: string s (mime.getContact());

		sendAck ( mime.getCSeq ( ), &v, &s );
	}
	return false;
}

bool SipLeg :: handleBye ( SIP :: PDU & mesg ) {
	const SIP :: MIMEInfo & mime = mesg.getMIME ( );
	if ( isPreviousHuntFrom ( mime ) )
		return true;
	thread -> released ( Q931 :: cvNormalCallClearing );
	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "OK" );
	insertPAssertId(mesg);
	sendMessage ( mesg, to_.getHostName ( ), to_.getPort ( ) );
	byeSent = true;
	return false;
}

bool SipLeg :: handleCancel ( SIP :: PDU & mesg ) {
	const SIP :: MIMEInfo & mime = mesg.getMIME ( );
	if ( isPreviousHuntFrom ( mime ) )
		return true;
	//thread -> released ( Q931 :: cvNoRouteToDestination );
    //thread -> released ( Q931 :: cvNoCircuitChannelAvailable );

    thread -> released ( conf -> getDefaultDisconnectCause() );

	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "OK" );
	insertPAssertId(mesg);

	if(isRecordRouteRequired())
		mesg.getMIME().setRoute(to_.getHostName ( ));// strContactRoute_);
	sendMessage ( mesg, to_.getHostName ( ), to_.getPort ( ) );
	return false;
}

bool SipLeg :: handleInvite ( SIP :: PDU & mesg ) {
	if ( mesg.hasSDP ( ) && mesg.getSDP ( ).getDefaultConnectAddress ( ).getAddr ( ) == "0.0.0.0" ) {
		thread -> onHold ( 7 );
		sendTrying ( mesg );
		return true;
	}
	return false;
}

void SipLeg :: sendTrying ( SIP :: PDU & mesg ) {
	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( SIP :: PDU :: scInformationTrying );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "Trying" );
	clearContent ( mesg );
	mesg.getMIME ( ).setContact ( "" );
	sendMessage ( mesg, to_.getHostName ( ), to_.getPort ( ) );

}

bool SipLeg :: handleUnAuthorised ( SIP :: PDU & mesg, int authType ) {
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	ss :: string authLine = "";
	switch ( authType ) {
		case SIP :: PDU :: scFailureUnAuthorised :
			authLine = mime.getWWWAuthenticate ( );
			break;
		case SIP :: PDU :: scFailureProxyAuthRequired :
			authLine = mime.getProxyAuthenticate ( );
			break;
		default :
			PSYSTEMLOG ( Error, "UnAuthorised: Unsupported statusCode " << authType );
			return false;
	}
	if ( authLine != "" ) {
		PSYSTEMLOG ( Info, "Requested auth on INVITE. Auth=" << mime.getWWWAuthenticate ( ) );
		StringVector v (mime.getRecordRoute ());
		ss :: string s (mime.getContact());
		sendAck ( changeCSeq ( mime.getCSeq ( ), "ACK" ), &v, &s );
		ss :: string gkIp = common.curChoice ( ) -> getIp ( );
		ss :: string gkLogin, gkPswd, gkCallId;
		int gkPort;
		if ( conf -> getGkInfo ( gkIp, gkLogin, gkPswd, gkPort ) ) {
			PSYSTEMLOG ( Info, "Sip: Destination is GK. Using GK params" );
			authLine = sipValidator.buildResponse ( authLine, gkLogin, gkPswd, mesg.getURI ( ).str ( ), "INVITE", true/*conf -> isUseCNonce( gkIp )*/ );
			sendInvite ( authLine, authType );
		} else {
			PSYSTEMLOG ( Info, "Sip: Destination isn't GK. Don't know how to authorise :(" );
			//thread -> released ( Q931 :: cvNoRouteToDestination );
            //thread -> released ( Q931 :: cvNoCircuitChannelAvailable );
            thread -> released ( conf -> getDefaultDisconnectCause() );
			return false;
		}
		return true;
	}
	return false;
}

bool SipLeg :: handleMessage ( SIP :: PDU & mesg ) {
    PSYSTEMLOG ( Info, "SipLeg :: handleMessage: New sip message : " << mesg );

	thread -> answered ( );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scInformationTrying )
		return handleTrying ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scInformationDialogEstablishement )
		return handleTrying ( mesg ); //voobshe neizvestnie nado obrabativat kak 183, no shas ne do etogo
	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scInformationRinging )
		return handleRinging ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scInformationSessionProgress )
		return handleSessionProgress ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scSuccessfulOK )
		return handleOK ( mesg );
	if ( ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scFailureUnAuthorised ) ||
	     ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scFailureProxyAuthRequired ) )
		return handleUnAuthorised ( mesg, mesg.getStatusCode ( ) );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) > SIP :: PDU :: scSuccessfulOK )
		return handleError ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mBye )
		return handleBye ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mCancel )
		return handleCancel ( mesg );
	if( mesg.getMethod ( ) == SIP :: PDU :: mMessage )
		PSYSTEMLOG(Info, "SipLeg: receive MESSAGE.");
	if ( mesg.getMethod ( ) == SIP :: PDU :: mInfo )
		return handleInfo ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mOptions )
		return handleOptions ( mesg );
	if ( mesg.getMethod ( ) == SIP :: PDU :: mInvite )
		return handleInvite ( mesg );
	PSYSTEMLOG ( Error, "unsupported message" );
	return false;
}

////////////////////////////////////////////////////////////////////////////////
ss :: string SipLeg :: createPAssertId(const SIP::URL & from1)
{
	ss :: string from = from1.str();

	ss :: string :: size_type startIndex = from.find("\"");
	ss :: string :: size_type endIndex = from.find("\"", startIndex+1);
	ss :: string :: size_type size(0);
	if(startIndex != ss :: string :: npos && endIndex != ss :: string :: npos ){
		size = endIndex - startIndex - 1;
	}
	ss :: string name(from.c_str() + (startIndex + 1), size);

	p_asserted_identity_ += "\"+";
	if(true == name.empty())
		name = from1.getUserName();
	p_asserted_identity_ += name;
	p_asserted_identity_ += "\" <sip:+";
	p_asserted_identity_ += from1.getUserName();
	p_asserted_identity_ += '@';
	p_asserted_identity_ += from1.getHostName();
	p_asserted_identity_ += '>';
        PSYSTEMLOG(Info, "2. SipLeg :: createPAssertId: " <<  p_asserted_identity_);
	return p_asserted_identity_;
}

void SipLeg :: insertPAssertId(SIP :: PDU & mesg)
{
	SIP::MIMEInfo & mime = mesg.getMIME ( );
//    CallsMap :: iterator it = calls.find ( callId );
 //   Pointer < SipCallDetails > call;
//	if(it != calls.end())
	{
//        call = it -> second;
//        CommonCallDetails & common = call -> common;
		const OutChoiceDetails * cur = common.curChoice ( );
		if(cur)
		{
	        bool pAssertReq = cur->getSigOptions ( ).isPAssertIdRequired();
	        PSYSTEMLOG(Info, "insertPAssertId: " <<  pAssertReq);
	        if(true == pAssertReq)
	        {
	            mime.setPAssertID(p_asserted_identity_);
	        }
		}
	}
}

bool SipLeg :: isRecordRouteRequired() const
{
	bool RRReq = false;
	{
		const OutChoiceDetails * cur = common.curChoice ( );
		if(cur)
		{
	        RRReq = cur->getSigOptions ( ).isRecordRouteRequired();
		}
	}
//	PSYSTEMLOG(Info, "SipLeg :: isRecordRouteRequired: " << RRReq);
	return RRReq;
//	return isRecordRouteRequired_;
}

bool SipLeg :: handleInfo( SIP :: PDU & mesg1 )
{
	PSYSTEMLOG(Info, "SipLeg :: handleInfo.");
	SIP :: PDU mesg;
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( via );
	mime.setFrom ( createAnonimousFromHeader(from_.str ( ), "\"Anonymous\" <sip:Anonymous@Anonymous.invalid>"));

	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setInfo ( "OK" );
	mime.setTo ( to_.str ( ) );
	mime.setCallID ( callId );

    SIP :: URL contact = mime.getFrom();
    contact.setHostName ( getLocalAddress ( ) );
    contact.setPort ( 5060 );
    mime.setContact ( contact.bracketShortForm ( ) );

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
	return sendMessage ( mesg, to_.getHostName ( ), to_.getPort ( ) );
}

bool SipLeg :: handleOptions ( SIP :: PDU & mesg1 )
{
	PSYSTEMLOG(Info, "SipLeg :: handleOptions.");
	SIP :: PDU mesg;
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setVia ( via );
	mime.setFrom ( createAnonimousFromHeader(from_.str ( ), "\"Anonymous\" <sip:Anonymous@Anonymous.invalid>"));

	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setInfo ( "OK" );
	mime.setTo ( to_.str ( ) );
	mime.setCallID ( callId );

    SIP :: URL contact = mime.getFrom();
    contact.setHostName ( getLocalAddress ( ) );
    contact.setPort ( 5060 );
    mime.setContact ( contact.bracketShortForm ( ) );

    mime.setCSeq ( mesg1.getMIME().getCSeq() );
	mime.setAccept ( "application/sdp" );
	mime.setMaxForwards ( 70 );
	mime.setUserAgent ( "PSBC SIP v2.0" );
	mime.setAcceptEncoding("identity");
	mime.setAllow( "INVITE, ACK, CANCEL, INFO, OPTIONS, BYE" );
	mime.setSupported("");
	return sendMessage ( mesg, to_.getHostName ( ), to_.getPort ( ) );
}


