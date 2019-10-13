#pragma implementation
#pragma implementation "sipcalldetails.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "ss.hpp"
#include "allocatable.hpp"
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include "pointer.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "codecinfo.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "sourcedata.hpp"
#include "outcarddetails.hpp"
#include "signallingoptions.hpp"
#include "outchoicedetails.hpp"
#include "moneyspool.hpp"
#include <tr1/functional>
#include "smartguard.hpp"
#include <tr1/memory>
#include "commoncalldetails.hpp"
#include "rtpstat.hpp"
#include "session.hpp"
#include "rtpsession.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include "ixcudpsocket.hpp"
#include "sip.hpp"
#include "callcontrol.hpp"
#include "SIPAuthenticateValidator.hpp"
#include "condvar.hpp"
#include <queue>
#include "automutex.hpp"
#include "threadmessagequeue.hpp"
#include "sipsignallingthread.hpp"
//------------
#include "sipcalldetails.hpp"
#include <ptlib/svcproc.h>
#include "Log.h"
#include "q931.hpp"
#include "legthread.hpp"
#include "h323call.hpp"
#include "answercall.hpp"
#include <stdexcept>
#include "mysql.hpp"
#include "sqloutthread.hpp"
#include "sipcommon.hpp"
#include "rasthread.hpp"
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include "serializestring.hpp"
#include "radiuscommon.hpp"
#include <cstring>

static const long EXPIRES_TIME = 180;

SipSignallingThread * sipThread = 0;

static ss :: string getLocalAddress  ( ) {
	PIPSocket :: Address addr = INADDR_ANY;
	if ( sipThread )
    {
        sipThread->GetInterfaceAddress(addr);
//        PSYSTEMLOG(Info, "getLocalAddress: sipThread = 1; addr = " << addr );
    }
    if ( addr == INADDR_ANY )
    {
    	PIPSocket :: GetHostAddress ( addr );
//        PSYSTEMLOG(Info, "getLocalAddress: sipThread = 0; addr = " << addr );
    }
	return static_cast < const char * > ( addr.AsString ( ) );
}



static void setToTerm ( SIP :: MIMEInfo & mime, const SipCallDetails * call, bool isInsertContact ) {
	mime.setVia ( call -> myVia );
	mime.setFrom ( call -> myFrom.str ( ) );
	mime.setTo ( call -> myTo.str ( ) );
	mime.setCallID ( call -> common.getConfId ( ) );

    if ( isInsertContact )
    {
        SIP :: URL contact = call -> myFrom.bracketShortForm ( );
        contact.setHostName ( :: getLocalAddress ( ) );
        contact.setPort ( 5060 );
        mime.setContact ( contact );
    }
}

static void setToTerm ( SIP :: PDU & pdu, const SipCallDetails * call, bool isInsertContact ) {
	setToTerm ( pdu.getMIME ( ), call, isInsertContact );
	pdu.setURI ( call -> termContact );
}


SipSignallingThread :: CallsMap SipSignallingThread :: calls;

void SipCallDetails :: beginShutDown ( ) {
	conf -> addSipCallToRemove ( common.getConfId ( ) );
}

int SipCallDetails :: getCallSeconds ( ) const {
	return conf -> getRounded ( double ( ( endTime - okTime ).GetMilliSeconds ( ) ) / 1000.0 );
}

bool SipCallDetails :: getCallConnected ( ) const {
	return oked;
}

int SipCallDetails :: getCallRef ( ) const {
	return id;
}

ss :: string SipCallDetails :: getInIp ( ) const {
	return common.getCallerIp ( );
}

int SipCallDetails :: getInPeerId ( ) const {
	return common.getSource ( ).peer;
}

ss :: string SipCallDetails :: getInPeerName ( ) const {
	return common.getSource ( ).name;
}

ss :: string SipCallDetails :: getInResponsibleName ( ) const {
	return common.getSource ( ).rname;
}

ss :: string SipCallDetails :: getSetupTime ( ) const {
	return static_cast < const char * > ( startTime.AsString ( ) );
}

ss :: string SipCallDetails :: getConnectTime ( ) const {
	if ( ! oked )
		return "-";
	return static_cast < const char * > ( okTime.AsString ( ) );
}

ss :: string SipCallDetails :: getOutIp ( ) const {
	const OutChoiceDetails * c = common.curChoice ( );
	if ( ! c )
		return "";
	return c -> getIp ( );
}

int SipCallDetails :: getOutPeerId ( ) const {
	const OutChoiceDetails * c = common.curChoice ( );
	if ( ! c )
		return 0;
	return c -> getPeer ( );
}

ss :: string SipCallDetails :: getCallId ( ) const {
	const ss :: string & confId = common.getConfId ( );
	return confId;
}

ss :: string SipCallDetails :: getOutPeerName ( ) const {
	return common.getOutName ( );
}

ss :: string SipCallDetails :: getOutResponsibleName ( ) const {
	return common.getOutRname ( );
}

ss :: string SipCallDetails :: getCallingDigits ( ) const {
	if ( common.getPeerIndex ( ) == -1 )
		return "-";
	else
		return common.getCallingDigits ( );
}

ss :: string SipCallDetails :: getSentDigits ( ) const {
	if ( common.getPeerIndex ( ) == -1 )
		return "-";
	else
		return common.getSentDigits ( );
}

ss :: string SipCallDetails :: getDialedDigits ( ) const {
	if ( common.getPeerIndex ( ) == -1 )
		return "-";
	else
		return common.getDialedDigits ( );
}

ss :: string SipCallDetails :: getInCode ( ) const {
	return common.getSource ( ).code;
}

ss :: string SipCallDetails :: getOutCode ( ) const {
	const OutChoiceDetails * c = common.curChoice ( );
	if ( ! c )
		return "";
	return c -> getCode ( ).substr ( c -> getDepth ( ) );
}

ss :: string SipCallDetails :: getInAcctn ( ) const {
	return common.getSource ( ).acctn;
}

ss :: string SipCallDetails :: getOutAcctn ( ) const {
	return common.getSource ( ).outAcctn;
}

ss :: string SipCallDetails :: getPdd ( ) const {
	PTime t = pddTime ? pddTime.get ( ) : PTime ( );
	ss :: ostringstream os;
	os << t - startTime;
	return os.str ( );
}

bool SipCallDetails :: getDifferentProtocol ( ) const {
	return false;
}

bool SipCallDetails :: getDifferentCodec ( ) const {
	return differentCodec;
}

const ss :: string & SipCallDetails :: getOrigInterface ( ) const {
	return origInterface;
}

const ss :: string & SipCallDetails :: getTermInterface (  ) const {
	return termInterface;
}

bool SipCallDetails :: getDirectRTP ( ) const {

//    PSYSTEMLOG( Info, "SipCallDetails :: getDirectRTP: common.getDirectIn = " <<  common.getDirectIn() <<
//                "; common.curChoice ( ) -> getDirectRTP = " << common.curChoice ( ) -> getDirectRTP() );

	return common.getDirectIn ( ) || common.curChoice ( ) -> getDirectRTP ( );
}

std :: set <ss :: string>  CompareCodecs :: m_alias729;
std :: set <ss :: string>  CompareCodecs :: m_annexB729;
bool CompareCodecs :: isInit = false;

void CompareCodecs :: initClassData()
{
    if(! isInit)
    {
        m_alias729.insert("g729r8");
        m_alias729.insert("g729ar8");
        m_alias729.insert("g729br8");
        m_alias729.insert("g729abr8");

        m_annexB729.insert("g729br8");
        m_annexB729.insert("g729abr8");

        isInit = true;
    }
}

bool CompareCodecs :: is729 (const ss :: string& codecName)
{
    initClassData();
    std :: set <ss :: string> :: const_iterator ii = m_alias729.find(codecName);
    return ii != m_alias729.end();
}

bool CompareCodecs :: isAnnexB (const ss :: string& codecName)
{
    initClassData();
    std :: set <ss :: string> :: const_iterator ii = m_annexB729.find(codecName);
    return ii != m_annexB729.end();
}

SipFastStartElement CompareCodecs :: getFastStartElement() const
{
//    PSYSTEMLOG(Info, "CompareCodecs: get " << m_Codec.codec.getCodec());
    return m_Codec;
}

CompareCodecs :: CompareCodecs(const SipCallDetails :: FastStartMap& mapFS, const SipFastStartElement& codec)
{
    //m_Codec = SipFastStartElement();
    ss :: string codecName = codec.codec.getCodec();
//    PSYSTEMLOG(Info, "CompareCodecs: Compare 0: " << codecName );
    initClassData();
    if(! is729(codecName))
    {
        SipCallDetails :: FastStartMap :: const_iterator ii = mapFS.find ( codecName );
        if( ii != mapFS.end ( ))
        {
            m_Codec = ii->second;
//            PSYSTEMLOG(Info, "CompareCodecs: Compare 1");
        }
        else
        {
            m_Codec = SipFastStartElement();
//            PSYSTEMLOG(Info, "CompareCodecs: Compare 2");
        }
    }
    else
    {
        for(SipCallDetails :: FastStartMap :: const_iterator ii = mapFS.begin( ); ii != mapFS.end ( ); ++ii)
        {
            if(is729(ii->first))
            {
                if(isAnnexB(ii -> first))
                {
                    m_Codec = codec;
//                    PSYSTEMLOG(Info, "CompareCodecs: Compare 3");

/*
                    if(isAnnexB(codec.codec.getCodec()))
                    {
                        m_Codec = ii->second;
                    }
                    else
                    {
                        m_Codec = codec;
//                        m_Codec.codec.setCodec(codecName);
                    }
*/
                }
                else
                {
                    if(isAnnexB(codecName))
                    {
                        m_Codec = SipFastStartElement();
//                        PSYSTEMLOG(Info, "CompareCodecs: Compare 4");
                        //m_Iterator = mapFS.end ( );
                    }
                    else
                    {
                        m_Codec = ii->second;
//                        PSYSTEMLOG(Info, "CompareCodecs: Compare 5");
                    }
                }
            }
        }
    }

//    PSYSTEMLOG(Info, "CompareCodecs: Compare " << m_Codec.codec.getCodec() << "; input = " << codecName);
};


SipReceiver :: SipReceiver ( PMutex & mut ) : m ( mut ), v ( mut ) { }

SipReceiver :: ~SipReceiver ( ) { }

bool SipReceiver :: get ( SIP :: PDU & mesg, int msec ) {
	AutoMutex am ( m );
	if ( messages.empty ( ) )
		v.Wait ( msec );
	if ( messages.empty ( ) )
		return false;
	mesg = messages.front ( );
	messages.pop_front ( );
	PSYSTEMLOG ( Info, "getting message from receiver " << this );
	return true;
}

void SipReceiver :: put ( const SIP :: PDU & mesg ) {
	AutoMutex am ( m );
	messages.push_back ( mesg );
	v.Signal ( );
}

void SipReceiver :: wake ( ) {
	AutoMutex am ( m );
	v.Signal ( );
}

SipReceiver * SipSignallingThread :: addReceiver ( const ss :: string & rid ) {
	AutoMutex am ( mut );

	if ( receivers.count ( rid ) )
		return 0;
	SipReceiver * r = new SipReceiver ( mut );
	receivers [ rid ] = r;
	PSYSTEMLOG ( Info, "adding receiver for " << rid << ": " << r );
	return r;
}

void SipSignallingThread :: removeReceiver ( const ss :: string & rid ) {
	PSYSTEMLOG ( Info, "removing receiver for " << rid );
	AutoMutex am ( mut );
	receivers.erase ( rid );
}

bool SipSignallingThread :: sendMessage ( SIP :: PDU & mesg, const PIPSocket :: Address & addr, int port ) {
//	PSYSTEMLOG( Info, "sendMessage: '" << addr << "', mess: \n\t" << mesg );
	AutoMutex am ( mut );
	deletePAssertId(mesg);
	Log -> LogSIPMsg ( mesg, OpengateLog :: Sending, addr, WORD ( port ), false );
	callerSocket -> SetSendAddress ( addr, WORD ( port ) );
	return mesg.write ( * callerSocket );
}

SipSignallingThread :: SipSignallingThread ( IxcUDPSocket * socket, unsigned _id ) :
	PThread ( 1000, NoAutoDeleteThread, NormalPriority, PString ( PString :: Printf,
	"SIP N%u", _id ) ), callerSocket ( socket ), id ( _id ), inviteTimerInterval_ ( EXPIRES_TIME ),
    isAccSend_( false ) {
	sipThread = this;
	myIP = :: getLocalAddress();
    //PSYSTEMLOG(Info, "!!!!!!!!!!!!!!!!!!!!!!!myIP = " << myIP);
	callerSocket -> SetReadTimeout ( 5000 );
	if ( ! callerSocket -> SetOption ( SO_KEEPALIVE, 1 ) )
		PSYSTEMLOG ( Error, "SO_KEEPALIVE: " << callerSocket -> GetErrorNumber ( ) << " " <<
			callerSocket -> GetErrorText ( ) );
	Resume ( );
}

void SipSignallingThread :: Close ( ) {
	// Later on this should be improved to do a proper cleanup of the call
	// rather than just dropping it....
	PSYSTEMLOG ( Info, "SipSignallingThread::Close ( )" );
	callerSocket -> Close ( );
}

static bool readMsg ( IxcUDPSocket * socket, SIP :: PDU & mesg ) {
	socket -> clear ( );
	bool ret = mesg.read ( * socket );
    return ret;
}

SipSignallingThread :: ~SipSignallingThread ( ) {
	sipThread = 0;
}

static void outputFastStartVectorData(const ss :: string& text, const SipFastStartElementVector& sfsev)
{
	PSYSTEMLOG(Info, text << ": size = " << sfsev.size());
	for(unsigned i = 0; i < sfsev.size(); ++i)
	{
		PSYSTEMLOG(Info, text << ": FastStart["<< i << "] = " << sfsev[i].format << ", " << sfsev[i].codec.getCodec ( ) );
	}
}
/*
static void outputSetCodecInfoData(const ss :: string& text, const SipFastStartElementVector& sfsev)
{
	for(unsigned i = 0; i < sfsev.size(); ++i)
	{
		PSYSTEMLOG(Info, text << ": FastStart["<< i << "] = " << sfsev[i].format << ", " << sfsev[i].codec.getCodec ( ) );
	}
}
*/

bool SipSignallingThread :: autorization(SIP :: PDU & mesg,
                                         PIPSocket ::Address& fromAddr,
                                         WORD& fromPort,
                                         ss :: string& username,
                                         bool &isCard)
{
    SIP :: MIMEInfo & mime = mesg.getMIME ( );

    PString t = fromAddr.AsString ( );
    ss :: string fromIp = static_cast < const char * > ( t );

    bool needAuthentication = true;
    if ( conf -> isValidInPeerAddress ( fromIp ) ) {
        PSYSTEMLOG ( Info, "(storm): INVITE from InPeer " << fromIp << ". Authentication skipped" );
        needAuthentication = false;
    }
    if(isCard)
    {
        needAuthentication = false;
    }
    if ( needAuthentication ) {
        ss :: string authLine = mime.getAuthorization ( );
        if ( authLine == "" ) {
            sendUnAuthorised ( mesg );
            return false;
        }
        ss :: string password;
        username = sipValidator.getLogin ( authLine );
        IpPortSet addresses;
        addresses.insert ( IpPort ( fromIp, fromPort ) );
        if ( ! conf -> getPassword ( PIPSocket :: Address ( myIP.c_str ( ) ),  addresses, username, password, isCard ) ) {
            PSYSTEMLOG ( Error, "SIPValidator : unknown username " << username );
            sendUnAuthorised ( mesg );
            return false;
        }
        if ( ! sipValidator.isValid ( authLine, "INVITE", password, true ) ) {
            PSYSTEMLOG ( Error, "SIPValidator : INVITE validation failed!" );
            sendUnAuthorised ( mesg );
            return false;
        }
        mime.setAuthorization ( "" );


        if ( conf -> getSipInviteRegistrationPeriod ( ) > 0 ) { // temporary registration for hunting
            IcqContactsVector icqContacts;
            StringSet onlineNumbers, neededNumbers;
            bool h323 = false;
            bool fromNat = true;
            PSYSTEMLOG ( Info, username << " temporary registered on INVITE for " << conf -> getSipInviteRegistrationPeriod ( ) << " sec" );
            conf -> registerPeer ( username, fromAddr, fromPort, h323, fromNat,
                PTime ( ) + conf -> getSipInviteRegistrationPeriod ( ) * 1000,
                isCard, fromAddr, fromNat ? fromPort : 5060, neededNumbers,
                onlineNumbers, 0, * ( PUDPSocket * ) 0, icqContacts );
        } else {
            PSYSTEMLOG ( Info, "Temporary registration on INVITE for " << username << " disabled" );
        }
    }
/*
	PSYSTEMLOG(Info, "SipSignallingThread :: autorization 100");

    return true;
=======
*/
	if ( conf -> isValidInPeerAddress ( fromIp ) ) {
		PSYSTEMLOG ( Info, "(storm): INVITE from InPeer " << fromIp << ". Authentication skipped" );
		return true;
	}
	if ( isCard )
		return true;
	ss :: string authLine = mime.getAuthorization ( );
	if ( authLine == "" ) {
		sendUnAuthorised ( mesg );
		return false;
	}
	ss :: string password;
	username = sipValidator.getLogin ( authLine );
	IpPortSet addresses;
	addresses.insert ( IpPort ( fromIp, fromPort ) );
	if ( ! conf -> getPassword ( PIPSocket :: Address ( myIP.c_str ( ) ),  addresses, username, password, isCard ) ) {
		PSYSTEMLOG ( Info, "SIPValidator : unknown username " << username );
		sendUnAuthorised ( mesg );
		return false;
	}
	if ( ! sipValidator.isValid ( authLine, "INVITE", password, true ) ) {
		PSYSTEMLOG ( Info, "SIPValidator : INVITE validation failed!" );
		sendUnAuthorised ( mesg );
		return false;
	}
	mime.setAuthorization ( "" );
	if ( conf -> getSipInviteRegistrationPeriod ( ) > 0 ) { // temporary registration for hunting
		IcqContactsVector icqContacts;
		StringSet onlineNumbers, neededNumbers;
		bool h323 = false;
		bool fromNat = true;
		PSYSTEMLOG ( Info, username << " temporary registered on INVITE for " <<
			conf -> getSipInviteRegistrationPeriod ( ) << " sec" );
		conf -> registerPeer ( username, fromAddr, fromPort, h323, fromNat,
			PTime ( ) + conf -> getSipInviteRegistrationPeriod ( ) * 1000,
			isCard, fromAddr, fromNat ? fromPort : 5060, neededNumbers,
			onlineNumbers, 0, * ( PUDPSocket * ) 0, icqContacts );
	} else
		PSYSTEMLOG ( Info, "Temporary registration on INVITE for " << username << " disabled" );
	return true;
}

static ss :: string parseToTag(const ss :: string& strFrom, bool isCoded)
{
	ss :: string :: size_type stBeginTag = strFrom.find("tag=");
	ss :: string strTag;
	if( ss :: string :: npos != stBeginTag )
	{
		strTag = strFrom.substr(stBeginTag + strlen("tag="),
								strFrom.size() - stBeginTag + strlen("tag=") + 1 );
	}
    if(isCoded)
        return strTag;
    else
        return toHex(strTag).substr( 0, 15 );
}

bool  SipSignallingThread :: isInviteFromTerminator(const SIP::PDU& newMesg, const SIP::PDU& oldMesg)
{
    ss :: string oldToTag = parseToTag(oldMesg.getMIME().getTo(), true);
    ss :: string newToTag = parseToTag(newMesg.getMIME().getTo(), true);
    ss :: string oldFromTag = parseToTag(oldMesg.getMIME().getFrom(), true);
    ss :: string newFromTag = parseToTag(newMesg.getMIME().getFrom(), true);

    if ( (oldToTag != newToTag      && oldFromTag == newToTag) &&
         (oldFromTag != newFromTag  && newFromTag == oldToTag) )
    {
        return false;
    }
    return true;
}

bool SipSignallingThread :: isRepitedInvite ( const ss :: string & callId, const ss :: string & strNewCSeq,
                                              bool isTerminatorsInvite ) {
	if ( strNewCSeq.empty ( ) )
		return false;
	if ( insertCSeq ( callId, strNewCSeq, isTerminatorsInvite ) )
		return true;
	MapOfSequences :: iterator it = mapOfSequences_.find ( callId );
    int newCSeq = std :: atoi ( strNewCSeq.c_str ( ) );
    int oldCSeq = -1;
    if(isTerminatorsInvite)
    {
        oldCSeq = it -> second.sequenceNumberTerminator_;
    }
    else
    {
        oldCSeq = it -> second.sequenceNumberOriginator_;
    }

    if ( oldCSeq < newCSeq) {
        if(isTerminatorsInvite)
        {
            it -> second.sequenceNumberTerminator_ = newCSeq;
        }
        else
        {
            it -> second.sequenceNumberOriginator_ = newCSeq;
        }
        PSYSTEMLOG(Info, "repited invite. secNum = " << newCSeq);
        return true;
    }

	return false;
}

bool SipSignallingThread :: insertCSeq ( const ss :: string & callId, const ss :: string & newCSeq,
                                         bool isTerminatorsInvite ) {
	MapOfSequences :: iterator it = mapOfSequences_.find ( callId );
	if ( it == mapOfSequences_.end ( ) ) {
		CallInternalData cid;

        if(isTerminatorsInvite)
        {
            cid.sequenceNumberTerminator_ = std :: atoi ( newCSeq.c_str ( ) );
        }
        else
        {
            cid.sequenceNumberOriginator_ = std :: atoi ( newCSeq.c_str ( ) );
        }

		mapOfSequences_.insert ( std :: make_pair ( callId, cid ) );
		return true;
	}
    else
    {
        if(isTerminatorsInvite)
        {
            it -> second.sequenceNumberTerminator_ = std :: atoi ( newCSeq.c_str ( ) );
        }
        else
        {
            it -> second.sequenceNumberOriginator_ = std :: atoi ( newCSeq.c_str ( ) );
        }
	}

    return false;
}

void SipSignallingThread :: deleteCSeq ( const ss :: string & callId ) {
	MapOfSequences :: iterator it = mapOfSequences_.find ( callId );
	if ( it != mapOfSequences_.end ( ) )
		mapOfSequences_.erase ( it );
}

class HuntThread : public PThread {
	PCLASSINFO(HuntThread, PThread);
	Pointer < SipCallDetails > call;
public:
	explicit HuntThread ( Pointer < SipCallDetails > call ) : PThread ( 1000, AutoDeleteThread, NormalPriority, "HuntThread" ),
		call ( call ) {
		Resume ( );
	}
	void Main ( ) {
		sipThread -> asyncHunting ( call );
		call = 0;
	}
};

bool SipSignallingThread :: handleInvite ( SIP :: PDU & mesg ) {
	if ( ! mesg.hasSDP ( ) )
		return false;
	PSYSTEMLOG ( Info, "SipSignallingThread :: handleInvite. CSeq = " << mesg.getMIME ( ).getCSeq ( ) );
	WORD fromPort = 5060;
	PIPSocket ::Address fromAddr;
	if ( ! handleViaFields ( mesg, fromAddr, fromPort ) )
		return false;

	bool isCard = false;
	PString t = fromAddr.AsString ( );
	ss :: string fromIp = static_cast < const char * > ( t );
	ss :: string username = "";
	if ( conf -> isRegisteredCardAddr ( fromIp, fromPort, username ) ) {
		PSYSTEMLOG ( Info, "(storm): INVITE from registered Card " << fromIp << ". Authentication skipped" );
		isCard = true;
	}/*
	else
	{
	
    if(! conf->isCardPresentAndRegister(dialed) )
    {
        PSYSTEMLOG(Info, "sendErrBack: SipSignallingThread :: handleInvite.3");
        call -> m_errorCode = SIP :: PDU :: scFailureNotFound; // 404
        sendErrBack ( call );
        accountCall ( call );
        return false;
    }
	}*/
	ss :: string newFullNum = mesg.getMIME().getTo();
	ss :: string newFromNum = mesg.getMIME().getFrom();
	bool isAnonymous = false;

	ss :: string oldNum = newFullNum;
	ss :: string addNum;
	ss :: string dummy;
	newFullNum = parsesAdditionalNumber ( oldNum, & dummy, & addNum );

    SipSignallingThread :: AdditionalNumber an = choiceAdditionalData ( addNum );
    PSYSTEMLOG ( Info, "HANDLE INVITE: TO: " << newFullNum );
	switch ( an ) {
		case SipSignallingThread :: anNone:
			newFromNum = mesg.getMIME ( ).getFrom ( );
			break;
		case SipSignallingThread :: anAnonymous:
			newFromNum = editNumberFromAnonymous ( );
            newFromNum += ";tag=";
            newFromNum += parseToTag(mesg.getMIME ( ).getFrom ( ), true);

			isAnonymous = true;
            PSYSTEMLOG ( Info, "HANDLE INVITE: isAnonymous = true. 0" << newFromNum );

		break;
    case SipSignallingThread :: anAnonymous1:
        newFromNum = mesg.getMIME ( ).getFrom ( );
        isAnonymous = true;
        PSYSTEMLOG ( Info, "HANDLE INVITE: isAnonymous = true. 1" << newFromNum );
        break;
	}
PSYSTEMLOG ( Info, "HANDLE INVITE: Before authentication" );
	// authentication
	if ( false == autorization ( mesg, fromAddr, fromPort, username, isCard ) ) {
		PSYSTEMLOG ( Error, "Authentication failed." );
		return false;
	}

    PSYSTEMLOG(Info, "INVITE: isCard = " << isCard << "; username = " << username << "; fromAddr = " << fromAddr << ":" << fromPort );
	// authentication OK

    bool bIsRepitedInvite = false;

    ss :: string callId = mesg.getMIME ( ).getCallID ( );
    CallsMap :: iterator it = calls.find ( callId );
    Pointer < SipCallDetails > call;
    SIP :: PDU trMesg = mesg;

    if ( it != calls.end ( ) ) {
//		PSYSTEMLOG ( Info, "Reinvite" );
		call = it -> second;
		if ( call -> oked )
            bIsRepitedInvite = true;
	}

    bool isTerminatorsInvite = false;
    if (call)
        isInviteFromTerminator(mesg, call -> invite);
	if ( false == isRepitedInvite ( mesg.getMIME ( ).getCallID (), mesg.getMIME ( ).getCSeq ( ), isTerminatorsInvite ) ) {
		PSYSTEMLOG(Error, "Repites sequence number. CSeq = " << mesg.getMIME ( ).getCSeq ( )
		<< ", CallId = " << mesg.getMIME ( ).getCallID ());
		return false;
	}

	SIP :: MIMEInfo & mime = mesg.getMIME ( );

	if ( it != calls.end ( ) ) {
		PSYSTEMLOG ( Info, "Reinvite" );
		call = it -> second;
		if ( call -> oked && isTerminatorsInvite )
			return handleReinviteFromOrig ( call, mesg );
        else
            return handleReinviteFromTerm ( call, mesg );
		return true;
	}

	PSYSTEMLOG ( Info, "Invite" );
	bool fromNat = true;
	sendTrying ( trMesg, mesg, fromAddr, fromPort );

	mesg.getMIME().setRoute("");

//	mesg.getMIME ( ).setTo ( newFullNum );
	conf -> take ( ); //deadlock - nado otvalivatsya pri limite
	PIPSocket :: Address local;
	GetInterfaceAddress ( local );
	call = new SipCallDetails ( local, fromAddr, fromNat, false );
	CommonCallDetails & common = call -> common;
	common.setCallerPort ( fromPort );
	common.setConfId ( callId );

	if ( SIP :: MediaDescription * media = mesg.getSDP().getMediaDescription ( SIP :: MediaDescription :: mtAudio ) )
    {
        if ( unsigned payload = media -> getTelephoneEventsPayloadType ( ) )
			common.setTelephoneEventsPayloadType ( payload );

        PIPSocket::Address addr;
        unsigned short port;
        media -> getTransportAddress ( ).getIpAndPort ( addr, port );
//        PSYSTEMLOG(Info, "~~~~~ handleInvite: " << addr << "; port = " << port << "; " << *media );
    }

	call -> invite = mesg;
	if ( conf -> isValidInPeerAddress ( common.getCallerIp ( ) ) )
		common.source ( ).type = SourceData :: inbound;
	if ( isCard ) {
		PSYSTEMLOG ( Info, "(storm): Forced to card " << username );
		common.source ( ).type = SourceData :: card;
		common.source ( ).acctn = username;
	}

    call -> from = mime.getFrom();
    PSYSTEMLOG(Info, "handleInvite: call -> from = " << call -> from );

	common.setCallingDigitsIn ( call -> from.getUserName ( ) );

    call -> to = mime.getTo ( );
    call -> contact = call -> to;
	call -> contact.setHostName ( :: getLocalAddress ( ) );
	call -> contact.setPort ( 5060 );

    ss :: string dialed;
    if( isAnonymous )
    {
        call -> isAnonymous_ = true;
        ss :: string newString = mesg.getURI ( ).getUserName ( );
        ss :: string :: size_type iBegNum = newString.find ( "*" );
        if ( iBegNum == ss :: string :: npos )
            dialed = newString;
        else
        {
            dialed = newString.substr(iBegNum + 3, ss :: string :: npos); // *67
        }
    }
    else
    {
        dialed = mesg.getURI ( ).getUserName ( );
    }
    call -> via = mime.getVia ( );
    call -> myFrom = newFromNum;//call -> from;


	common.setDialedDigits ( dialed );
	CodecInfoVector incomingCodecs;
	SIP :: SessionDescription & sdp = mesg.getSDP ( );

	getIncomingCodecsFromSDP ( sdp, common.incomingCodecs ( ) );
	call -> vRecordRoute_ = mime.getRecordRoute ( );

	if ( ! mime.updateMaxForwards ( mime ) ) {
		PSYSTEMLOG(Info, "sendErrBack: SipSignallingThread :: handleInvite.1");
		sendErrBack ( call );
		accountCall ( call );
		return false;
	}
	if ( SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) )
		media -> getTransportAddress ( ).getIpAndPort ( call -> origRtpAddr, call -> origRtpPort );

	conf -> addActiveCall ( call -> id, call );
	bool useAsyncHunting = true;
	if ( useAsyncHunting ) {
		common.setAnonymous ( isAnonymous );
		new HuntThread ( call );
		return true;
	}
/*
    if(! conf->isCardPresentAndRegister(dialed) )
    {
        PSYSTEMLOG(Info, "sendErrBack: SipSignallingThread :: handleInvite.3");
        call -> m_errorCode = SIP :: PDU :: scFailureNotFound; // 404
        sendErrBack ( call );
        accountCall ( call );
        return false;
    }
*/
	if ( ! conf -> getCallInfo ( call -> choiceForks,
		call -> forkOutAcctns, common, true, true, true ) ) {
		PSYSTEMLOG(Info, "sendErrBack: SipSignallingThread :: handleInvite.2");
		sendErrBack ( call );
		accountCall ( call );
		return false;
	}
	mime.setRecordRouteRequired ( isRecordRouteRequired ( call ) );

	PSYSTEMLOG( Info,
                "INVITE: is empty incomming codecs = " << !!( common.getIncomingCodecs ( ).c.empty ( ) ) <<
                "; hasH323 = " << !!(hasH323Choices ( common.getChoices( ) )) <<
                "; choiceForks.size = " << call -> choiceForks.size ( ) <<
                "; useNormalizer = " << common.getUseNormalizer ( ) );

    if ( ! common.getIncomingCodecs ( ).c.empty ( ) && ( hasH323Choices ( common.getChoices( ) ) ||
		call -> choiceForks.size ( ) > 1 || common.getUseNormalizer ( ) ) ) {
		conf -> removeActiveCall ( call -> id );
		common.setAnonymous ( isAnonymous );
		PIPSocket :: Address sdpAddr;
    		ss :: string passertFrom = mesg.getMIME().getPAssertID();
	        setPAssertId(mesg.getMIME ( ).getCallID ( ), passertFrom );
		insertPAssertId(mesg);
					
		new AnswerCall ( call -> choiceForks, call -> forkOutAcctns, call, myIP );
		return true;
	}


	calls.insert ( std :: make_pair ( common.getConfId ( ), call ) );
	call -> inviteTemp = mesg;
	if ( tryNextChoice ( call ) )
		return true;
	sendErrBack ( call );
	accountCall ( call );
	calls.erase ( common.getConfId ( ) );
	return false;
}

void SipSignallingThread :: asyncHunting ( Pointer < SipCallDetails > call ) {
	CommonCallDetails & common = call -> common;
	if ( ! conf -> getCallInfo ( call -> choiceForks, call -> forkOutAcctns, common, true, true, true ) ) {
		PSYSTEMLOG(Info, "sendErrBack: SipSignallingThread :: handleInvite.");
		sendErrBack ( call );
		q.put ( std :: tr1 :: bind ( & SipSignallingThread :: accountCall, this, call ) );
		return;
	}
	PSYSTEMLOG( Info,
                "INVITE: is empty incomming codecs = " << !!( common.getIncomingCodecs ( ).c.empty ( ) ) <<
                "; hasH323 = " << !!(hasH323Choices ( common.getChoices( ) )) <<
                "; choiceForks.size = " << call -> choiceForks.size ( ) <<
                "; useNormalizer = " << common.getUseNormalizer ( ) );

	if ( ! common.getIncomingCodecs ( ).c.empty ( ) ) {
		conf -> removeActiveCall ( call -> id );
		PIPSocket :: Address sdpAddr;
		new AnswerCall ( call -> choiceForks, call -> forkOutAcctns, call, myIP );
		return;
	}
	sendErrBack ( call );
	q.put ( std :: tr1 :: bind ( & SipSignallingThread :: accountCall, this, call ) );
}

static void closeThisChoice ( SipCallDetails * call ) {
	call -> differentCodec = false;
	const CommonCallDetails & common = call -> common;
	call -> tries.push_back ( OutTry ( common.getPeerIndex ( ), common.getDisconnectCause ( ), common.getConfId ( ) ) );
}

bool SipSignallingThread :: tryNextChoice ( SipCallDetails * call ) {
    PSYSTEMLOG( Info, "SipSignallingThread :: tryNextChoice. to = " << call -> to );

    if(! call)
        return false;
	CommonCallDetails & common = call -> common;
	if ( common.getPeerIndex ( ) >= 0 )
		closeThisChoice ( call );
	if ( call -> goodCall )
		return false;
	if ( conf -> shuttingDown ( ) )
		return false;
	if ( call -> outTaken ) {
		conf -> release ( common, true, false );
		call -> outTaken = false;
	}
	bool limited = false;
	int limitedCause = conf -> getDefaultDisconnectCause(); /*Q931 :: cvNoCircuitChannelAvailable;*/
/*
    PSYSTEMLOG(Info, "tryNextChoice: dialed: " << call -> getDialedDigits() );
    if ( conf -> cardByNumber( call -> getDialedDigits() ) )
    {
        bool isCard = conf ->isRegisteredCardAddr(common.curChoice ( ) -> getIp ( ),
                                                  WORD ( common.curChoice ( ) -> getPort ( ) ),
                                                  accountCard_);
        PSYSTEMLOG(Info, "tryNextChoice: isCard: " << isCard );
        if( ! isCard )
        {
            call -> m_errorCode = SIP :: PDU :: scFailureNotFound; // 404
            PSYSTEMLOG(Info, "tryNextChoice: ret: " << call -> m_errorCode );
//            return false;
        }
    }
    PSYSTEMLOG(Info, "tryNextChoice: after dialed: " << call -> m_errorCode );
*/
	while ( common.hasNextPeerIndex ( ) ) {
		common.nextPeerIndex ( );
		if ( ( common.curChoice ( ) -> getIp ( ) == common.getCallerIp ( ) ) &&
			( common.curChoice ( ) -> getPort ( ) == common.getCallerPort ( ) ) )
			continue;
		common.sigOptions ( ).setOut ( common.curChoice ( ) -> getSigOptions ( ) );
        if(common.getSigOptions ( ). isAccSend())
        {
            bool isCard = conf ->isRegisteredCardAddr(common.curChoice ( ) -> getIp ( ),
                                                      WORD ( common.curChoice ( ) -> getPort ( ) ),
                                                      accountCard_);
            if(isCard)
            {
                SIP :: URL to = call -> to;
                to.setUserName( accountCard_ );
                call -> origCardName = call -> to;
                call -> termCardName = to;
                isAccSend_ = true;
//                PSYSTEMLOG(Info, "SipSignallingThread :: tryNextChoice: orig to: " << call -> origCardName.str() << "; term to" <<  call -> termCardName.str());
            }
        }
        else
        {
            call -> termCardName = call -> origCardName = call -> to;
        }
		if ( ! conf -> take ( common, limitedCause, 0 ) ) {
			limited = true;
			PSYSTEMLOG ( Error, "Limit reached: " << common.getCalledIp ( ) );
			continue;
		}

        {
//            conf -> setAllowedRouteRTP(common, call ->getInPeerId() );
        }
        call -> outTaken = true;
		limited = false;
		limitedCause = conf -> getDefaultDisconnectCause(); /* Q931 :: cvNoCircuitChannelAvailable;*/
		if(false == sendInviteForward ( call ))
		{
			PSYSTEMLOG(Error, "Common (In-peer and Out-peer) codecs is absent. Try next choice.");
			continue;
		}
		return true;
	}
	if ( limited )
	{	common.setDisconnectCause ( limitedCause );
        call -> m_errorCode = SIP :: PDU :: scFaiureServiceUnavailable;
    }

    PSYSTEMLOG(Info, "tryNextChoice: end: " << call -> m_errorCode );
	return false;
}

static ss :: string translateTo ( const ss :: string & to, const ss :: string & newTo ) {
	if ( ( to.find ( ';' ) != ss :: string :: npos ) || ( newTo.find ( ';' ) == ss :: string :: npos ) )
		return to;
	return to + newTo.substr ( newTo.find ( ';' ) );
}

static void getRtpCodec ( const SIP :: MediaFormat & format, int & rtpCodec ) {
	rtpCodec = 0;
	switch ( format.getPayloadType ( ) ) {
		case SIP :: MediaFormat :: PCMA:
		case SIP :: MediaFormat :: PCMU:
			rtpCodec = 2;
			break;
		case SIP :: MediaFormat :: G7231:
			rtpCodec = 4;
			break;
		case SIP :: MediaFormat :: G729:
			rtpCodec = 3;
		default:
			break;
	}
}

//PIPSocket :: Address SipSignallingThread :: getLocalAddress ( SipCallDetails * /*call*/, bool /*callerSide*/ ) {
//  return PString ( myIP.c_str ( ) );
//}

static void setFrames ( SIP :: MediaFormat & format, int setFrames ) {
	switch ( format.getPayloadType ( ) ) {
		case SIP :: MediaFormat :: G729:
		case SIP :: MediaFormat :: PCMA:
		case SIP :: MediaFormat :: PCMU:
			format.setPtime ( setFrames * 10 );
			break;
		case SIP :: MediaFormat :: G7231:
			format.setPtime ( setFrames * 30 );
		default:
			break;
	}
}

bool SipSignallingThread :: handleFastStartResponse ( SipCallDetails * call, SIP :: MediaDescription * media ) {
//	PSYSTEMLOG ( Info, "SipSignallingThread :: handleFastStartResponse");
	SIP :: MediaFormatVector fastStart = media -> getMediaFormats ( );
	SipFastStartElementVector fsin, fsout;
	for ( SIP :: MediaFormatVector :: const_iterator i = fastStart.begin ( ); i != fastStart.end ( ); ++ i ) {
		fsin.push_back ( SipFastStartElement ( * i ) );
		fsout.push_back ( SipFastStartElement ( * i ) );
	}
	if ( fsin.size ( ) < 1 || fsout.size ( ) < 1 )
		return false;
	SipFastStartElement & ie = fsin [ 0 ];

//    PSYSTEMLOG(Info, "CompareCodecs: input ");
    CompareCodecs compCodecs(call -> originalFastStartIn, ie);
    SipFastStartElement ii = compCodecs.getFastStartElement();
    if ( ii.codec.getCodec() == "unknown" )
    {
        return false;
    }
/*
	SipCallDetails :: FastStartMap :: const_iterator ii = call -> originalFastStartIn.find ( ie.codec.getCodec ( ) );

    if ( ii == call -> originalFastStartIn.end ( ) ) {
        for(ii = call -> originalFastStartIn.begin ( ); ii != call -> originalFastStartIn.end ( ); ++ii)
        {
            PSYSTEMLOG(Info, "Codecs: " << ii -> first << "; " << ii -> second . codec.getCodec());
        }
        PSYSTEMLOG(Info, "Search codec: " << ie.codec.getCodec ( ));

		return false;
	}
*/
	SipFastStartElement & oe = fsout [ 0 ];
/*
    SipCallDetails :: FastStartMap :: iterator oi = call -> originalFastStartOut.find ( oe.codec.getCodec ( ) );

    if ( oi == call -> originalFastStartOut.end ( ) ) {
		return false;
	}
*/
//    PSYSTEMLOG(Info, "CompareCodecs: output ");
    CompareCodecs compCodecs1(call -> originalFastStartOut, oe);
    SipFastStartElement oi = compCodecs1.getFastStartElement ();
    if ( oi.codec.getCodec() == "unknown" )
    {
        return false;
    }

	call -> codec = ie.codec;
	const SipFastStartElement & oie = ii;
	Session * s = call -> session;
	fastStart.clear ( );
	bool direct = call -> getDirectRTP ( );
	if ( ! direct ) {
//        PSYSTEMLOG(Info, "handleFastStartResponse: 1" );
		ss::string localAddr = call -> getOrigInterface ( );
		int localRtpPort = s -> getLocalAddress ( false, false );
		PIPSocket :: Address addr;
		WORD port;
		media -> getTransportAddress ( ).getIpAndPort ( addr, port );
        //PSYSTEMLOG(Info, "handleFastStartResponse: setSendAddress " << addr << "; \n" << *media );
		s -> setSendAddress ( false, true, addr, WORD ( port + 1 ), 0, 0, false, "", "" );

		media -> setTransportAddress ( SIP :: TransportAddress ( localAddr, WORD ( localRtpPort ) ) );
		int rtpCodec = 0, rtpFrames = oie.changeFrames;
		bool sendCodec = rtpFrames;
        //PSYSTEMLOG(Info, "handleFastStartResponse: 2" );

		if ( sendCodec ) {
			getRtpCodec ( ie.format, rtpCodec );
			sendCodec = rtpCodec;
		}
		ss :: string recodeTo, recodeFrom;
		if ( oie.recodeTo.getCodec ( ) != "unknown" )
		{
			recodeFrom = oie.recodeTo.getCodec ( );
		}
		if ( oie.codec.getCodec ( ) != "unknown" )
		{
			recodeTo = oie.codec.getCodec ( );
		}
        PSYSTEMLOG(Info, "~~SipSignallingThread :: handleFastStartResponse: setSendAddr: localAddr=" << localAddr <<
               "; addr=" << addr << "; port=" << port << "; recodeFrom=" <<  recodeFrom << "; recodeTo=" << recodeTo);
        	call -> differentCodec = isDifferentCodec ( recodeTo, recodeFrom );
		s -> setSendAddress ( false, false, addr, port,
			rtpCodec, rtpFrames, sendCodec, recodeTo, recodeFrom );
	}
	const SipFastStartElement & ooe = oi;
	oe.format = ooe.format;
	if ( ! direct ) {
		if ( ooe.changeFrames )
			setFrames ( oe.format, ooe.changeFrames );
		s -> setSendAddress ( true, true, call -> origRtpAddr, WORD ( call -> origRtpPort + 1 ), 0, 0, false, "", "" );
		int rtpCodec = 0, rtpFrames = ooe.changeFrames;
		bool sendCodec = rtpFrames;
		if ( sendCodec ) {
			getRtpCodec ( oe.format, rtpCodec );
			sendCodec = rtpCodec;
		}
		ss :: string recodeTo, recodeFrom;
		if ( ooe.recodeTo.getCodec ( ) != "unknown" ) {
			recodeFrom = ooe.codec.getCodec ( );
			recodeTo = ooe.recodeTo.getCodec ( );
			PSYSTEMLOG ( Info, "From codec: " << recodeFrom );
			PSYSTEMLOG ( Info, "To codec: " << recodeTo );
		}
//        PSYSTEMLOG(Info, "handleFastStartResponse: 3" );
        	call -> differentCodec = isDifferentCodec ( recodeTo, recodeFrom );
		s -> setSendAddress ( true, false, call -> origRtpAddr, call -> origRtpPort, rtpCodec, rtpFrames,
			sendCodec, recodeTo, recodeFrom );
//        PSYSTEMLOG(Info, "handleFastStartResponse: 4" );
	}

//    PSYSTEMLOG(Info, "handleFastStartResponse: 5 media: \n" << (*media) );

	fastStart.push_back ( oe.format );
	for(unsigned i = 1; i < fsout .size(); ++i )
	{
		SipFastStartElement & oe = fsout [ i ];
		if(oe.format.getEncodingName() == "telephone-event")
		{
			fastStart.push_back ( oe.format );
			break;
		}
	}
	if ( unsigned payload = media -> getTelephoneEventsPayloadType ( ) )
		s -> setTelephoneEventsPayloadType ( payload );
	media -> setMediaFormats ( fastStart );
	return true;
}

class IPParser
{
public:
    IPParser (const ss :: string& ip);

    bool operator == (const IPParser& parserTest)
    {
	bool retVal = false;
	if(parserTest.m_ip1 != "256")
	{
	    retVal = parserTest.m_ip1 == m_ip1;
	    if( retVal && parserTest.m_ip2 != "256")
	    {
		retVal = parserTest.m_ip2 == m_ip2;
		if(retVal && parserTest.m_ip3 != "256")
		{
		    retVal = parserTest.m_ip3 == m_ip3;
		    if(retVal && parserTest.m_ip4 != "256")
		    {
			retVal = parserTest.m_ip4 == m_ip4;
		    }
		}
	    }
	}

	return retVal;
    }

protected:
    ss :: string m_ip;

    ss :: string m_ip1;
    ss :: string m_ip2;
    ss :: string m_ip3;
    ss :: string m_ip4;
};

IPParser :: IPParser(const ss :: string& ip)
{
	ss :: string :: size_type pos = ip.find(".");
        if(pos != ss :: string :: npos)
        {
		m_ip1 = ip.substr ( 0, pos );
		ss :: string :: size_type pos1 = ip.find(".", pos+1);
		if(pos1 != ss :: string :: npos)
		{
			m_ip2 = ip.substr ( pos+1, pos1 );
			ss :: string :: size_type pos2 = ip.find(".", pos1+1);
			if(pos2 != ss :: string :: npos)
			{
				m_ip3 = ip.substr ( pos1+1, pos2 );
				ss :: string :: size_type pos3 = ip.find(".", pos2+1);
				if(pos3 != ss :: string :: npos)
				{
					m_ip4 = ip.substr ( pos2+1, pos3 );
				}
//				else
//				{
//					PSYSTEMLOG(Info, "IPParser :: IPParser(): 4th part of ip adress is absent" << ip );
//				}
			}
//			else
//			{
//				PSYSTEMLOG(Info, "IPParser :: IPParser(): 3th part of ip adress is absent" << ip );
//			}
		}
//		else
//		{
//			PSYSTEMLOG(Info, "IPParser :: IPParser(): 2nd part of ip adress is absent" << ip );
//		}
	}
//	else
//	{
//		PSYSTEMLOG(Info, "IPParser :: IPParser(): first part of ip adress is absent" << ip );
//	}
}

static bool isPrivateIP(const ss :: string& IPAddr)
{
//Check if the remote address is a private IP address. RFC 1918 specifies the following private IP addresses
//10.0.0.0    - 10.255.255.255.255
//172.16.0.0  - 172.31.255.255
//192.168.0.0 - 192.168.255.255
//??? ????????????? ??????????? IP-??????? ?????????? Internet Assigned Numbers Authority (IANA)
// ?????????????? ???????? ???????
// 169.254.0.0-169.254.255.255.
    IPParser parser(IPAddr);

    return (	parser == IPParser("10.256.256.256")
	     || parser == IPParser("172.16.31.256")
	     || parser == IPParser("192.168.256.256")
	     || parser == IPParser("127.0.0.1")
	     || parser == IPParser("169.254.256.256") );
}

void SipSignallingThread :: handleAnswerSDP ( SIP :: PDU & mesg, SipCallDetails * call, const ss :: string & myIP ) {
    PSYSTEMLOG ( Info, "SipSignallingThread :: handleAnswerSDP");

    if ( mesg.hasSDP ( ) ) {
		SIP :: SessionDescription & sdp = mesg.getSDP ( );
		if ( ! call -> getDirectRTP ( ) ) {
			sdp.setDefaultConnectAddress ( myIP );
			sdp.setOwnerAddress ( myIP );
		}
		if ( SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) )
		{
            if( ! handleFastStartResponse ( call, media ) )
            {
                sendCancelForward1(call);
                PSYSTEMLOG ( Info, "handleAnswerSDP: send Cancel");
            }
        }
	}
}

void SipSignallingThread :: sendTrying ( SIP :: PDU & mesg, SIP :: PDU & inviteMesg,
										 PIPSocket ::Address fromAddr, WORD fromPort ) {
//	PIPSocket ::Address fromAddr;
//	WORD fromPort;
//	callerSocket -> GetLastReceiveAddress ( fromAddr, fromPort );
	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( SIP :: PDU :: scInformationTrying );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "Trying" );
/*
	{
		ss :: ostringstream os;
		os << "SIP/2.0/UDP " << getLocalAddress() << ":5060";
		ss :: string myVia = os.str ( );
		ss :: string oldVia = mesg.getMIME().getVia();
		ss :: string via = myVia;
		via += "\n";
		via += oldVia;

		mesg.getMIME().setVia(via);
	}
*/
	clearContent ( mesg );
    mesg.getMIME().setContact("");
	if(isRecordRouteRequired(inviteMesg))
	{
		mesg.getMIME().setRecordRouteRequired(isRecordRouteRequired(mesg));
		StringVector vRecord = inviteMesg.getMIME().getRecordRoute();
		for(unsigned i = 0; i < vRecord.size(); ++i)
		{
			mesg.getMIME().addRecordRoute(vRecord[i], false);
		}

		mesg.getMIME().addRecordRoute(getLocalAddress());
	}

//	PSYSTEMLOG(Info, "SipSignallingThread :: sendTrying: Addr = " << fromAddr << "; port = " << fromPort);
	sendMessage ( mesg, fromAddr, fromPort );
}

void SipSignallingThread :: sendByeForward ( const SipCallDetails * call ) {
//	PSYSTEMLOG(Info, "SipSignallingThread :: sendByeForward IsRRReq = " << call->isRecordRouteRequired_);
	SIP :: PDU mesg = call -> inviteTemp;
	mesg.setMIME ( SIP :: MIMEInfo ( isRecordRouteRequired ( call ) ) );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	setToTerm ( mesg, call, true );
	mime.setCSeq ( "101 BYE" );
	mesg.setMethod ( SIP :: PDU :: mBye );
	clearContent ( mesg );
//	if(isRecordRouteRequired(const_cast < SipCallDetails * >(call)))
//	if(call->isRecordRouteRequired_)

	mime.setRoute(call -> myTo.getHostName ( ));
	sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
}

void SipSignallingThread :: sendCancelForward1 ( const SipCallDetails * call ) {
	SIP :: PDU mesg = call -> inviteTemp;
	mesg.setMIME ( SIP :: MIMEInfo ( isRecordRouteRequired ( call ) ) );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	setToTerm ( mesg, call, true );
	mime.setCSeq ( "102 CANCEL" );
	mesg.setMethod ( SIP :: PDU :: mCancel );
	clearContent ( mesg );
/*
	if(isRecordRouteRequired(inviteMesg))
	{
		mesg.getMIME().setRecordRouteRequired(isRecordRouteRequired(mesg));
		StringVector vRecord = inviteMesg.getMIME().getRecordRoute();
		for(unsigned i = 0; i < vRecord.size(); ++i)
		{
			mesg.getMIME().addRecordRoute(vRecord[i], false);
		}

		mesg.getMIME().addRecordRoute(getLocalAddress());
	}
*/
	mesg.getMIME().addRecordRoute(getLocalAddress());

	sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
}

static void setToOrig ( SIP :: MIMEInfo & mime, const SipCallDetails * call, bool isInsertContact ) {
	mime.setVia ( call -> myVia );
	mime.setFrom ( call -> to.str ( ) );
	mime.setTo ( call -> from.str ( ) );
	mime.setCallID ( call -> common.getConfId ( ) );
    if (isInsertContact)
    {
        mime.setContact ( call -> to.bracketShortForm ( ) );
    }
}

static void setToOrig ( SIP :: PDU & pdu, const SipCallDetails * call, bool isInsertContact ) {
	setToOrig ( pdu.getMIME ( ), call, isInsertContact );
//	pdu.setURI ( pdu.getMIME ( ).getTo ( ) );
	pdu.setURI ( call -> invite.getMIME ( ).getContact ( ) );
}

void SipSignallingThread :: sendByeBackward ( const SipCallDetails * call ) {
	SIP :: PDU mesg = call -> inviteTemp;
	mesg.setMIME ( SIP :: MIMEInfo ( isRecordRouteRequired ( call ) ) );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	setToOrig ( mesg, call, true );
	mime.setCSeq ( "101 BYE" );
	mesg.setMethod ( SIP :: PDU :: mBye );
	clearContent ( mesg );
//	if(isRecordRouteRequired(const_cast < SipCallDetails * >(call)))
//	if(call->isRecordRouteRequired_)
		mime.setRoute(call -> common.getCallerIp ( ));
	sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ),
		call -> common.getCallerPort ( ) );
}

static bool isPreviousHunt ( const SIP :: URL & u1, const SIP :: URL & u2 ) {
	if ( u1.getHostName ( ) != u2.getHostName ( ) || u1.getUserName ( ) != u2.getUserName ( ) ||
		u1.getPort ( ) != u2.getPort ( ) ) {
		PSYSTEMLOG ( Error, "From previous hunt" );
		return true;
	}
	return false;
}

static bool isPreviousHuntToTerm ( const SIP :: MIMEInfo & mime, const SipCallDetails * call ) {
	return isPreviousHunt ( mime.getTo ( ), call -> myTo );
}

static bool isPreviousHuntFromTerm ( const SIP :: MIMEInfo & mime, const SipCallDetails * call ) {
	return isPreviousHunt ( mime.getFrom ( ), call -> myTo );
}

bool SipSignallingThread :: handleTrying ( SIP :: PDU & mesg ) {
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
		return false;
	Pointer < SipCallDetails > call = it -> second;
	if ( isPreviousHuntToTerm ( mime, call ) )
		return false;
	const ss :: string & contact = mime.getContact ( );
	if ( ! contact.empty ( ) )
		call -> termContact = contact;
	call -> answered = true;
	return true;
}

static void setFromOrig ( SIP :: MIMEInfo & mime, const SipCallDetails * call, bool isInsertContact ) {
	mime.setVia ( call -> via );
	mime.setFrom ( call -> from.str ( ) );
	mime.setTo ( call -> to.str ( ) );
    if ( isInsertContact )
        mime.setContact ( call -> contact.bracketShortForm ( ) );
}

static void setFromTerm ( SIP :: MIMEInfo & mime, const SipCallDetails * call, bool isInsertContact ) {
	mime.setVia ( call -> calledVia );
	mime.setFrom ( call -> myTo.str ( ) );
	mime.setTo ( call -> myFrom.str ( ) );
//	mime.setContact ( call -> myFrom.bracketShortForm ( ) );
    if ( isInsertContact)
    {
        SIP :: URL contact = call -> myFrom.bracketShortForm ( );
        contact.setHostName ( :: getLocalAddress ( ) );
        contact.setPort ( 5060 );
        mime.setContact ( contact );
    }
}

bool SipSignallingThread :: handleOK ( SIP :: PDU & mesg ) {
	PSYSTEMLOG(Info, "SipSignallingThread :: handleOK.");
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
//	if ( mime.getCSeq ( ).find ( "MESSAGE" ) != ss :: string :: npos )
//		return handleMessageOK ( mesg );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
	{
		PSYSTEMLOG(Info, "SipSignallingThread :: handleOK. calls.size = " << calls.size());
		return false;
	}
	SipCallDetails * call = it -> second;
	call -> goodCall = true;

	if ( isRecordRouteRequired ( call ) ) {
		mesg.getMIME().clearRecordRoute();
		mesg.getMIME().setRecordRouteRequired(true);
		const StringVector& vRecord = call -> vRecordRoute_;
		for(StringVector :: size_type j = vRecord.size ( ); j --; )
		{
			if(getLocalAddress() != vRecord[j])
				mesg.getMIME().addRecordRoute(vRecord[j], false);
		}
		mesg.getMIME().addRecordRoute(getLocalAddress());
	}

	if ( mime.getCSeq ( ).find ( "CANCEL" ) != ss :: string :: npos )
		return handleCancelOK ( mesg );
	if ( mime.getCSeq ( ).find ( "BYE" ) != ss :: string :: npos )
		return handleByeOK ( mesg );
// Hack for Marin John. Global URI support in OK
//	if ( isPreviousHuntToTerm ( mime, call ) )
//		return false;

	const ss :: string & contact = mime.getContact ( );
	if ( ! contact.empty ( ) )
		call -> termContact = contact;
	if ( ! call -> oked && ! call -> removed ) {
		call -> oked = true;
		call -> okTime = PTime ( );
		call -> goodCall = true;
		call -> answered = true;
		ss :: ostringstream os;
		const CommonCallDetails & common = call -> common;
		os << "replace ActiveCalls set id = " << call -> id << ", startTime = from_unixtime( "
			<< PTime ( ).GetTimeInSeconds ( ) << " ), setupTime = startTime, inIP = '" <<
			common.getCallerIp ( ) << "', inCallId = '" << common.getConfId ( ) << "', outCallId = '" <<
			common.getConfId ( ) << "', convertedDigits = '" << common.getConvertedDigits ( ) << "', outIP = '"
			<< call -> myTo.getHostName ( ) << "', outName = '" << common.getOutName ( ) << "', outRname = '"
			<< common.getOutRname ( )  <<
			"', connectTime = from_unixtime( " <<
			PTime ( ).GetTimeInSeconds ( ) << " ), priceCache = '";
		ss :: ostringstream ot;
		const OutTryVector & tries = call -> tries;
		{
			boost :: archive :: text_oarchive oa ( ot );
			oa << common << tries;
		}
		os << MySQL :: escape ( ot.str ( ) ) << "'";
		sqlOut -> add ( os.str ( ) );
		conf -> addCall ( call );
	}
	SIP :: URL str = mime.getTo ( );

    SIP :: URL oldMyTo = call -> myTo;
    //call -> myTo = str;

    if ( ! isAccSend_ )
    {
        call -> to = translateTo ( call -> to.str ( ), str.str() );
        PSYSTEMLOG(Info, "SipSignallingThread :: handleOK. isAccSend = false. to = " << call -> to);
    }
    else
    {
        //call -> myTo.setUserName( call -> common.getSentDigits ( ) );
        //call -> to = str;
        //call -> to.setUserName( call -> common.getSentDigits ( ) );
        //call -> to.setHostName( getLocalAddress ( ) );
        str.setHostName(call -> origCardName.getHostName());
        str.setPort(call -> origCardName.getPort());
        str.setUserName(call -> origCardName.getUserName());
        call -> origCardName = str;

        str.setHostName(call -> myTo.getHostName());
        str.setPort(call -> termCardName.getPort());
        str.setUserName(call -> termCardName.getUserName());
        call -> termCardName = str;

        call -> to = call -> origCardName;
        call -> myTo = str;//call -> termCardName;
        PSYSTEMLOG(Info, "SipSignallingThread :: handleOK. isAccSend = true. to = " << call -> to << "; myTo = " << call -> myTo);
    }
    PSYSTEMLOG(Info, "SipSignallingThread :: handleOK: call -> from = " << call -> from );

	setFromOrig ( mime, call, true );
	handleAnswerSDP ( mesg, call, call->getOrigInterface() );
// Hack for re-invite handler, we don't want to change CSeq in it. Orig
// is not overwrited by ReINVITE.
	mime.setCSeq ( call -> origCSeq );

	call -> vOKRecordRoute_ = mime.getRecordRoute ();
	if ( ! call -> pddTime )
		call -> pddTime = PTime ( );
	sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ),
		call -> common.getCallerPort ( ) );
    call -> myTo = oldMyTo;
	return true;
}

bool SipSignallingThread :: handleMessageOK ( SIP :: PDU & mesg )
{
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	SIP :: URL tmpTo = mime.getTo ( );
	SIP :: URL tmpFrom = mime.getFrom ( );

	//setFromTerm ( mime, call );
	//mime.setVia ( call -> calledVia );
	mime.setTo ( tmpFrom.str ( ) );
	mime.setFrom ( tmpTo.str ( ) );
	mime.setContact ( tmpFrom.bracketShortForm ( ) );
	sendMessage ( mesg, PIPSocket :: Address ( tmpTo.getHostName ( ).c_str ( ) ), tmpTo.getPort ( ) );
	return true;
}

bool SipSignallingThread :: handleCancelOK ( SIP :: PDU & mesg ) {
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
		return false;
	SipCallDetails * call = it -> second;
	SIP :: URL tmp = mime.getTo ( );
	bool fromCaller = ! ( call -> myTo.getUserName ( ) == tmp.getUserName ( ) &&
		call -> myTo.getHostName ( ) == tmp.getHostName ( ) );
//	PSYSTEMLOG ( Info, "fromCaller: " << fromCaller );
    SIP :: URL oldMyTo = call -> myTo;
	if ( fromCaller ) {
		if ( SIP :: URL ( mime.getTo ( ) ).getHostName ( ) != call -> from.getHostName ( ) ) {
			PSYSTEMLOG ( Error, "reply from previous hunt" );
			return false;
		}
/*
        if ( isAccSend_ )
        {
*/
/*
            call -> to = tmp;
            call -> to.setUserName( accountCard_ );
            call -> to.setHostName( getLocalAddress() );
*/
/*
            call -> myTo = call -> to = call -> origCardName;
            //call -> myTo.setUserName(  call -> common.getSentDigits ( ) );
            PSYSTEMLOG(Info, "handleCancelOk. Term. to = " << call -> to.str() << "; myTo = " << call -> myTo.str() );
        }
*/
		setFromTerm ( mime, call, true );
		sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
	} else {
		if ( isPreviousHuntToTerm ( mime, call ) )
			return false;
/*
        if ( isAccSend_ )
        {
*/
/*
            call -> to = tmp;
            call -> to.setUserName(  call -> common.getSentDigits ( ) );
            call -> to.setHostName( getLocalAddress() );
            call -> myTo.setUserName(  call -> common.getSentDigits ( ) );
*/
/*
            call -> myTo = call -> to = call -> termCardName;
            //call -> myTo.setUserName(  call -> common.getSentDigits ( ) );
            PSYSTEMLOG(Info, "SipSignallingThread :: handleCancelOk. Orig. to = " << call -> to.str() << "; myTo = " << call -> myTo.str() );
        }
*/
		setFromOrig ( mime, call, true );
//		mime.setCSeq ( call -> origCSeq );
		sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ),
			call -> common.getCallerPort ( ) );
        call -> myTo = oldMyTo;
	}
	return true;
}

bool SipSignallingThread :: handleByeOK ( SIP :: PDU & mesg ) {
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );

	if ( it == calls.end ( ) )
		return false;
	SipCallDetails * call = it -> second;
	if ( call -> killed )
		return true;
	SIP :: URL tmp = mime.getTo ( );
	bool fromCaller = ! ( call -> myTo.getUserName ( ) == tmp.getUserName ( ) &&
		call -> myTo.getHostName ( ) == tmp.getHostName ( ) );
	PSYSTEMLOG ( Info, "handleByeOK. fromCaller: " << fromCaller << "; myTo = " << call -> myTo.str()
                 << "; mime.getTo = " << mime.getTo() );
    SIP :: URL oldMyTo = call -> myTo;

	if ( fromCaller ) {
// Fix for global URI support in responces. From URI might have other hostname then our IP(To field).
//	  if ( SIP :: URL ( mime.getTo ( ) ).getHostName ( ) != call -> from.getHostName ( ) ) {
//		  PSYSTEMLOG ( Error, "reply from previous hunt" );
//		  return false;
//	  }
/*
        if ( isAccSend_ )
        {
*/
/*
            call -> from = tmp;
            call -> to.setUserName( accountCard_ );
            call -> to.setHostName( getLocalAddress() );
            call -> myTo.setUserName( accountCard_ );
*/
/*
            call -> myTo = call -> to = call -> origCardName;
            //call -> myTo.setUserName(  call -> common.getSentDigits ( ) );
            PSYSTEMLOG(Info, "SipSignallingThread :: handleByeOK. isAccSend = true. Term. to = " << call -> to.str() << "; myTo = " << call -> myTo.str() );
        }
*/
		setFromTerm ( mime, call, true );
		sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
	} else { // from terminator
		if ( isPreviousHuntToTerm ( mime, call ) )
			return false;
/*
        if ( isAccSend_ )
        {
*/
/*
            call -> to.setUserName( call -> common.getSentDigits ( ) );
            call -> to.setHostName( getLocalAddress() );
*/
/*
            call -> myTo = call -> to = call -> termCardName;
//            call -> myTo.setUserName( call -> common.getSentDigits ( ) );
            PSYSTEMLOG(Info, "SipSignallingThread :: handleByeOK. isAccSend = true. Orig. from = " << call -> from.str() << "; myFrom = " << call -> myFrom.str() );
        }
*/
		setFromOrig ( mime, call, true );
//	  mime.setCSeq ( call -> origCSeq );

        PIPSocket :: Address sendAddr ( call -> common.getCallerIp ( ).c_str ( ) );
        PSYSTEMLOG(Info, "SipSignallingThread :: handleByeOK. to = " <<  sendAddr << "; port = " << call -> common.getCallerPort ( ) );
		sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ),
			call -> common.getCallerPort ( ) );
	}
    call -> myTo = oldMyTo;
	return true;
}

bool SipSignallingThread :: handleAck ( SIP :: PDU & mesg ) {
/*
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
		return false;
	Pointer < SipCallDetails > call = it -> second;
    //SIP :: URL oldMyTo = call -> myTo;*/
/*
    if ( isAccSend_ )
    {
        call -> myTo = call -> to = call -> termCardName;

//        call -> to = call -> myTo;
//        call -> to.setUserName( accountCard_ );
//        call -> to.setHostName( getLocalAddress() );
 //       call -> myTo = call -> to;
        PSYSTEMLOG(Info, "SipSignallingThread :: handleAck. isAccSend = true. to = " << call -> to << "; from = " << call -> from);
    }
*/
	//setToTerm ( mesg, call, false /*true*/ ); // V tech-invite.com net polya CONTACT v Ack.
/*
    if ( isAccSend_ )
    {
        mime.setTo ( call -> to.str ( ) );
    }
*/
    /*
	SIP :: URL tmp = mime.getTo ( );

	mime.setRecordRouteRequired(isRecordRouteRequired( call ));
	if(true == mime.getRecordRoute ().empty() && false == call -> vOKRecordRoute_.empty())
	{
		StringVector recordRoute(call -> vOKRecordRoute_);
//		reverse(recordRoute.begin(), recordRoute.end());
		recordRoute.pop_back();
		if(false == recordRoute.empty())
		{
			mime.setRoute(recordRoute);
		}
	}
//    call -> myTo = oldMyTo;
    //sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
    sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
	return true;
    */
       SIP :: MIMEInfo & mime = mesg.getMIME ( );
        CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
        if ( it == calls.end ( ) )
                return false;
        Pointer < SipCallDetails > call = it -> second;
        setToTerm ( mesg, call, false );
        SIP :: URL tmp = mime.getTo ( );

        mime.setRecordRouteRequired(isRecordRouteRequired( call ));
        if(true == mime.getRecordRoute ().empty() && false == call -> vOKRecordRoute_.empty())
        {
                StringVector recordRoute(call -> vOKRecordRoute_);
//              reverse(recordRoute.begin(), recordRoute.end());
                recordRoute.pop_back();
                if(false == recordRoute.empty())
                {
                        mime.setRoute(recordRoute);
                }
        }
        sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
        //sendMessage ( mesg, PIPSocket :: Address ( call -> common.curChoice ( ) -> getIp ( ).c_str() ),
        //              call -> common.curChoice ( ) -> getPort() );
        return true;
}

bool SipSignallingThread :: handleBye ( SIP :: PDU & mesg ) {
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
		return false;
	SipCallDetails * call = it -> second;
	if ( ! call -> byed ) {
		call -> byed = true;
		call -> endTime = PTime ( );
		conf -> removeCall ( call );
		deleteCSeq(mime.getCallID ( ));
	}
	insertPAssertId(mesg);
	SIP :: URL tmp = mime.getFrom ( );
	bool fromCaller = call -> from.getUserName ( ) == tmp.getUserName ( ) &&
		call -> from.getHostName ( ) == tmp.getHostName ( );
	PSYSTEMLOG ( Info, "fromCaller: " << fromCaller << "; from = " << call -> from.str() );
    SIP :: URL oldMyTo = call -> myTo;
    if ( fromCaller ) {
/*
        if ( isAccSend_ )
        {
*/
            //call -> myFrom.setUserName ( call -> common.getSentDigits ( )/*call -> common.getCallingDigits ( )*/ );
/*
            PSYSTEMLOG(Info, "SipSignallingThread :: handleBye. from Orig befor. to = " <<
                       call -> to.str() << "; myFrom = " << call -> myFrom.str() << "; MyTo = " << call -> myTo << "; accountCard_ = " << accountCard_ );

            call -> to = call -> myTo;
            call -> to.setUserName( accountCard_ );
            call -> myTo = call -> to;
*/
/*
            call -> myTo = call -> to = call -> termCardName;

            PSYSTEMLOG(Info, "SipSignallingThread :: handleBye. isAccSend = true. Term. to = " << call -> to.str() << "; myFrom = " << call -> myFrom.str() );
        }
*/
        setToTerm ( mesg, call, false );
        call -> myTo = oldMyTo;
        mime.setRoute(call -> myTo.getHostName ( ));
		sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
	} else
    { // Bye from Terminator
		call -> calledVia = mime.getVia ( );
/*
        if ( isAccSend_ )
        {
*/
/*
            PSYSTEMLOG(Info, "SipSignallingThread :: handleBye. Befor to = " << call ->to.str() << "; from = " << call -> from.str() );
            //call -> to = tmp;
            call -> to.setUserName( call -> common.getSentDigits ( ) );
            call -> to.setHostName( getLocalAddress() );
*/
/*
            call -> myTo = call -> to = call -> origCardName;

            PSYSTEMLOG(Info, "SipSignallingThread :: handleBye. isAccSend = true.  Orig. to = " << call -> to.str() << "; myFrom = " << call -> myFrom.str() );
        }
*/
		setToOrig ( mesg, call, true );
// Sat. It's not good to send original CSeq from INVITE on BYE.
//	  mime.setCSeq ( call -> origCSeq );

        call -> myTo = oldMyTo;
        mime.setRoute(call -> common.getCallerIp ( ));
		sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ),
			call -> common.getCallerPort ( ) );
	}
	return true;
}

bool SipSignallingThread :: handleRinging ( SIP :: PDU & mesg ) {
	PSYSTEMLOG(Error, "SipSignallingThread :: handleRinging");
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
		return false;
	Pointer < SipCallDetails > call = it -> second;
	if ( isPreviousHuntToTerm ( mime, call ) )
		return false;
	call -> goodCall = true;
	call -> answered = true;
	SIP :: URL str = mime.getTo ( );
//    SIP :: URL oldMyTo = call -> myTo;
//    call -> myTo = str;

    if ( ! isAccSend_ )
    {
        call -> to = translateTo ( call -> to.str ( ), str.str() );
    }
    else
    {
/*
        call -> myTo.setUserName( call -> common.getSentDigits ( ) );
        call -> to = str;
        call -> to.setUserName( call -> common.getSentDigits ( ) );
        call -> to.setHostName ( getLocalAddress ( ) );
*/
        str.setHostName(call -> origCardName.getHostName());
        str.setPort(call -> origCardName.getPort());
        str.setUserName(call -> origCardName.getUserName());
        call -> origCardName = str;

        str.setHostName(call -> myTo.getHostName());
        str.setPort(call -> termCardName.getPort());
        str.setUserName(call -> termCardName.getUserName());
        call -> termCardName = str;

        call -> to = call -> origCardName;
        call -> myTo = str; // call -> termCardName;

        PSYSTEMLOG(Error, "SipSignallingThread :: handleRinging. isAccSend = true. to = " << call -> to);
    }

	const ss :: string & contact = mime.getContact ( );
	if ( ! contact.empty ( ) )
		call -> termContact = contact;
	setFromOrig ( mime, call, true );
	handleAnswerSDP ( mesg, call, call->getOrigInterface() );
	mime.setCSeq ( call -> origCSeq );
	if ( isRecordRouteRequired ( call ) ) {
		mesg.getMIME().clearRecordRoute();
		mesg.getMIME().setRecordRouteRequired(true);
		const StringVector& vRecord = call -> vRecordRoute_;
		for(StringVector :: size_type j = vRecord.size ( ); j --; )
		{
			if(getLocalAddress() != vRecord[j])
				mesg.getMIME().addRecordRoute(vRecord[j], false);
		}

		mesg.getMIME().addRecordRoute(getLocalAddress());
		mesg.getMIME().setRoute("");
	}
	if ( ! call -> pddTime )
		call -> pddTime = PTime ( );
//    call -> myTo = oldMyTo;
	sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ),
		call -> common.getCallerPort ( ) );
	return true;
}

void SipSignallingThread :: sendUnAuthorised ( SIP :: PDU & mesg ) {
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	PIPSocket ::Address fromAddr;
	WORD fromPort;
//	PSYSTEMLOG(Info, "1sendUnAuthorised: \n" << mesg);
//	clearContent ( mesg );
//	PSYSTEMLOG(Info, "2sendUnAuthorised: \n" << mesg);

	ss :: string strFrom = mime.getFrom();
	ss :: string strTo = mime.getTo();
	ss :: ostringstream os;
	ss :: string strTag = parseToTag(strFrom, false);
	if(strTag.empty())
	{
		os << strTo;
		PSYSTEMLOG(Info, "SipSignallingThread :: sendUnAuthorised(): without Tag = " << os.str() );
	}
	else
	{
		ss :: string :: size_type begTag = strTo.find(";tag=");
		if(begTag == ss :: string :: npos)
		{
			os << strTo << ";tag=" << strTag;
			PSYSTEMLOG(Info, "SipSignallingThread :: sendUnAuthorised(): new Tag = " << os.str() );
		}
		else
		{
			os << strTo;
			PSYSTEMLOG(Info, "SipSignallingThread :: sendUnAuthorised(): old Tag = " << os.str() );
		}
	}
	mime.setTo ( os.str() );

	callerSocket -> GetLastReceiveAddress ( fromAddr, fromPort );
	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( SIP :: PDU :: scFailureUnAuthorised );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "Unauthorised" );
	ss :: string authRequest;
	sipValidator.composeRequest ( authRequest );
	mime.setWWWAuthenticate ( authRequest );
	mime.setAuthorization ( "" );
	sendMessage ( mesg, fromAddr, fromPort );
}

bool SipSignallingThread :: handleRegister ( SIP :: PDU & mesg ) {
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	PIPSocket::Address fromAddr;
	WORD fromPort = 5060;

	if(!handleViaFields(mesg, fromAddr, fromPort))
	{
		return false;
	}

	PString t = fromAddr.AsString ( );
	ss :: string fromIp = static_cast < const char * > ( t );


	bool isCard = false;
	ss :: string username = "";
	ss :: string expires = mime.getExpires ( );
	if (  expires == "" ) {
		SIP :: URL contact = mime.getContact ( );
		expires = contact.getSipParam ( "expires" );
	}
	if ( expires == "" ) {
		ss :: ostringstream os;
		os << conf -> getSipDefaultRegistrationPeriod ( );
		expires = os.str ( );
	}
	long expiresT = std :: atol ( expires.c_str ( ) ) * 1000;
	bool registrationRequired = true;
    bool isRegisteredCard = conf -> isRegisteredCardAddr ( fromIp, fromPort, username );
	if ( expiresT == 0 && isRegisteredCard ) { // UNREGISTER from known card
		PSYSTEMLOG ( Info, "SIP UNREGISTER from known card " << username << ", authentication skipped" );
		registrationRequired = false;
	}
	if ( conf -> isValidInPeerAddress ( fromIp ) && ! isRegisteredCard ) { // authentication not required for InPeers
		if ( conf -> isValidOutPeerAddress ( fromIp, fromPort ) ) {
			PSYSTEMLOG ( Info, "SIP REGISTER. Looks like floating OutPeer. Always need registration" );
			registrationRequired = true;
		} else {
			PSYSTEMLOG ( Info, "SIP REGISTER. Looks like InPeer (" << fromAddr.AsString ( ) << "), authentification skipped" );
			registrationRequired = false;
		}
	}
	if ( registrationRequired ) {
		ss :: string authLine = mime.getAuthorization ( );
		if ( authLine == "" ) {
			sendUnAuthorised ( mesg );
			return false;
		}
		username = sipValidator.getLogin ( authLine );
		ss :: string password = "";
		IpPortSet addresses;
		addresses.insert ( IpPort ( fromIp, fromPort ) );
		if ( ! conf -> getPassword ( PIPSocket :: Address ( myIP.c_str ( ) ),  addresses, username, password, isCard ) ) {
			PSYSTEMLOG ( Info, "SIPValidator : unknown username " << username );
			sendUnAuthorised ( mesg );
			return false;
		}
		if ( ! sipValidator.isValid ( authLine, "REGISTER", password, true ) ) {
			PSYSTEMLOG ( Info, "SIPValidator : REGISTER validation failed" );
			sendUnAuthorised ( mesg );
			return false;
		}
		mime.setAuthorization ( "" );
	}
	// authentication OK
	// registering
	Conf :: UnregisteredContactsMap unregisteredContacts;
	IcqContactsVector icqContacts;
	StringSet onlineNumbers;
	StringSet neededNumbers;

//  PUDPSocket sock;

//  PIPSocket ::Address fromAddr;
//  WORD fromPort;
//  callerSocket -> GetLastReceiveAddress ( fromAddr, fromPort );
	bool h323 = false;
	bool fromNat = true;
	if ( expiresT != 0 ) {
		conf -> registerPeer ( username, fromAddr, fromPort, h323, fromNat,
				PTime ( ) + expiresT, isCard, fromAddr, fromNat ? fromPort : 5060, neededNumbers,
				onlineNumbers, & unregisteredContacts, * ( PUDPSocket * ) 0, icqContacts );
	} else {
		conf -> unregisterInPeer ( fromAddr, fromPort, unregisteredContacts );
	}
	rasThread -> sendNotifications ( unregisteredContacts );
	// registration OK
	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "OK" );
	mime.setExpires ( expires );
	PSYSTEMLOG(Info, "Register: expires = " << expires);

	if(isPrivateIP(fromIp))
	{
//		PIPSocket::Address fromAddr1;
		WORD fromPort1;

		callerSocket -> GetLastReceiveAddress ( fromAddr, fromPort1 );
//		PString t = fromAddr1.AsString ( );
// 		fromIp = static_cast < const char * > ( t );

	}


	sendMessage ( mesg, fromAddr, fromPort );
	return true;
}

bool SipSignallingThread :: handleOptions ( SIP :: PDU & mesg ) {
	PIPSocket::Address fromAddr;
	WORD fromPort = 5060;

	if(!handleViaFields(mesg, fromAddr, fromPort))
	{
		return false;
	}

	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "OK" );
	sendMessage ( mesg, fromAddr, fromPort );
	return true;
}

bool SipSignallingThread :: sendAck ( SIP :: PDU & mesg ) {
//	PSYSTEMLOG(Info, "SipSignallingThread :: sendAck");
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mesg.setMethod ( SIP :: PDU :: mAck );
	SIP :: URL to = mime.getTo ( );
	mesg.setURI ( to );
	mime.setCSeq ( changeCSeq ( mime.getCSeq ( ), "ACK" ) );

	ss :: string auth = mime.getWWWAuthenticate ( );
	mime.setWWWAuthenticate ( "" );

//	mime.setRecordRouteRequired(isRecordRouteRequired(mesg));
//	if(isRecordRouteRequired(mesg))
		mime.setRoute(to.getHostName ( ));

	sendMessage ( mesg, PIPSocket :: Address ( to.getHostName ( ).c_str ( ) ), to.getPort ( ) );
	mime.setWWWAuthenticate ( auth );
	return true;
}

//void SipSignallingThread :: sendErrBack ( SIP :: PDU & mesg ) {
//  /*
//  SIP :: MIMEInfo & mime = mesg.getMIME ( );
//  ss :: string ip;
//  if ( ! getIpFromVia ( mime.getVia ( ), ip ) ) {
//	  PSYSTEMLOG ( Error, "cannot get ip from via" );
//	  return;
//  }
//  */
//  SIP :: MIMEInfo & mime = mesg.getMIME ( );
//  PIPSocket ::Address fromAddr;
//  WORD fromPort;
//  callerSocket -> GetLastReceiveAddress ( fromAddr, fromPort );
//  mesg.setMethod ( SIP :: PDU :: mNum );
//  mesg.setStatusCode ( SIP :: PDU :: scFailureForbidden );
//  mesg.setURI ( SIP :: URL ( ) );
//  mesg.setInfo ( "Forbidden" );
//  clearContent ( mesg );
//  sendMessage ( mesg, fromAddr, fromPort );
//}

void SipSignallingThread :: sendErrBack ( const SipCallDetails * call ) {
	SIP :: PDU mesg = call -> invite;
//  SIP :: MIMEInfo & mime = mesg.getMIME ( );
//  PIPSocket ::Address fromAddr;
//  WORD fromPort;
//  callerSocket -> GetLastReceiveAddress ( fromAddr, fromPort );
	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( call -> m_errorCode /*SIP :: PDU :: scFaiureServiceUnavailable*/ /*scFailureForbidden*/ );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "Service Unavailable." );
	clearContent ( mesg );
	sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ),
		call -> common.getCallerPort ( ) );
}

void SipSignallingThread :: sendErrMessage ( const SIP :: PDU & mesg1, const CommonCallDetails & common )
{
	SIP :: PDU mesg = mesg1;

	mesg.setMethod ( SIP :: PDU :: mNum );
	mesg.setStatusCode ( SIP :: PDU :: scAccepted );
	mesg.setURI ( SIP :: URL ( ) );
	mesg.setInfo ( "Accepted" );
	clearContent ( mesg );
	sendMessage ( mesg, PIPSocket :: Address ( common.getCallerIp ( ).c_str ( ) ),
		common.getCallerPort ( ) );
}


bool SipSignallingThread :: handleCancel ( SIP :: PDU & mesg ) {
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
		return false;
	SipCallDetails * call = it -> second;
	if ( ! call -> cancelled ) {
		deleteCSeq(mime.getCallID ( ));
		call -> cancelled = true;
		call -> endTime = PTime ( );
		conf -> removeCall ( call );
	}
	SIP :: URL tmp = mime.getFrom ( );
	bool fromCaller = call -> from.getUserName ( ) == tmp.getUserName ( ) &&
		call -> from.getHostName ( ) == tmp.getHostName ( );
//	PSYSTEMLOG ( Info, "fromCaller: " << fromCaller );
	if ( fromCaller ) {
/*
        if ( isAccSend_ )
        {
            call -> to = tmp;
            call -> to.setUserName( accountCard_ );
            call -> to.setHostName( getLocalAddress() );
            call -> myTo.setUserName( accountCard_ );
            //call -> myTo =  call -> to.bracketShortForm() ;
            PSYSTEMLOG(Info, "SipSignallingThread :: handleCancel. Orig. to = " << call -> to.str() << "; myTo = " << call -> myTo.str() );
        }
*/
		setToTerm ( mesg, call, false );
		sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
	} else {
		if ( isPreviousHuntFromTerm ( mime, call ) )
			return false;
/*
        if ( isAccSend_ )
        {
            call -> to = tmp;
            call -> to.setUserName(  call -> common.getSentDigits ( ) );
            //call -> to.setUserName( accountCard_ );
            call -> to.setHostName( getLocalAddress() );
            call -> myTo.setUserName(  call -> common.getSentDigits ( ) );
            //call -> myTo =  call -> to.bracketShortForm() ;
            PSYSTEMLOG(Info, "SipSignallingThread :: handleCancel. Term. to = " << call -> to.str() << "; myTo = " << call -> myTo.str() );
        }
*/
		call -> calledVia = mime.getVia ( );
		setToOrig ( mesg, call, false );
//		mime.setCSeq ( call -> origCSeq );
		sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ),
			call -> common.getCallerPort ( ) );
	}
	return true;
}

bool SipSignallingThread :: handleUnAuthorised ( SIP :: PDU & mesg, int authType ) {
//	PSYSTEMLOG(Info, "handleUnAuthorised");
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
		return false;
	SipCallDetails * call = it -> second;
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
	if (  authLine != "" ) {
		PSYSTEMLOG ( Info, "Requested auth on INVITE. Auth=" << mime.getWWWAuthenticate ( ) );
		sendAck ( mesg );
		ss :: string gkIp = call -> common.curChoice ( ) -> getIp ( );
		ss :: string gkLogin, gkPswd, gkCallId;
		int gkPort;
		if ( conf -> getGkInfo ( gkIp, gkLogin, gkPswd, gkPort ) ) {
			PSYSTEMLOG ( Info, "Sip: Destination is GK. Using GK params" );
			authLine = sipValidator.buildResponse ( authLine, gkLogin, gkPswd, "sip:" + myIP, "INVITE",
				isCNonce ( & call -> common ) );
			sendInviteForward ( call, authLine, authType );
		} else {
			PSYSTEMLOG ( Info, "Sip: Destination isn't GK. Don't know how to authorise :(" );
			sendErrBack ( call );
		}
		return true;
	}
	return false;
}


bool SipSignallingThread :: handleError ( SIP :: PDU & mesg ) {
    PSYSTEMLOG ( Info, "SipSignallingThread :: handleError: " << mesg );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
	{
        PSYSTEMLOG ( Info, "handleError. it == calls.end ( ). CallID = " << mime.getCallID ( ) );
        return false;
    }
	SipCallDetails * call = it -> second;
	SIP :: URL tmp = mime.getTo ( );
	bool fromCaller = ! ( call -> myTo.getUserName ( ) == tmp.getUserName ( ) &&
		call -> myTo.getHostName ( ) == tmp.getHostName ( ) );
	PSYSTEMLOG ( Info, "handleError: fromCaller: " << fromCaller );
	if ( fromCaller ) {
		if ( SIP :: URL ( mime.getTo ( ) ).getHostName ( ) != myIP ) {
/*
            if ( isAccSend_ )
            {
                PSYSTEMLOG ( Error, "handleError: " << mime.getTo ( )  << "; " << myIP );
                if ( isAccSend_ )
                {
                    call -> from = tmp;
                    call -> to.setUserName( accountCard_ );
                    call -> to.setHostName( getLocalAddress() );
                    call -> myTo.setUserName( accountCard_ );
                    PSYSTEMLOG(Info, "handleError. isAccSend = true. Orig. to = " << call -> to.str() << "; myTo = " << call -> myTo.str() );
                }
            }
            else
            {*/
                PSYSTEMLOG ( Error, "reply from previous hunt" );
                return false;
            //}
		}
		setFromTerm ( mime, call, false );
		sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
	} else {
/*
        if ( isAccSend_ )
        {
            call -> to = tmp;
            call -> to.setUserName(  call -> common.getSentDigits ( ) );
            call -> to.setHostName( getLocalAddress() );
            call -> myTo.setUserName(  call -> common.getSentDigits ( ) );
            PSYSTEMLOG(Info, "handleError. Term. to = " << call -> to.str() << "; myTo = " << call -> myTo.str() );
        }
*/
        if ( isPreviousHuntToTerm ( mime, call ) )
        {
            PSYSTEMLOG ( Error, "isPreviousHuntToTerm. return false" );
        	return false;
        }
		if ( tryNextChoice ( call ) )
		{
            PSYSTEMLOG ( Error, "tryNextChoice. return true" );
            return true;
        }
		setFromOrig ( mime, call, false );
		mime.setCSeq ( call -> origCSeq );
		if ( ! call -> pddTime )
			call -> pddTime = PTime ( );
		sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ),
			call -> common.getCallerPort ( ) );
	}
	call -> errored = true;
	call -> errorTime = PTime ( );
//  accountCall ( call );
//  calls.erase ( it );
    PSYSTEMLOG ( Error, "handleError: After sendMessage. return true" );
	return true;
}

void SipSignallingThread :: accountCall ( SipCallDetails * call ) {
	call -> session = 0;
	ss :: ostringstream os;
	if ( call -> outTaken )
		conf -> release ( call -> common, true, false );
	if ( call -> tries.empty ( ) )
		call -> common.setPeerIndex ( - 1 );
	else
		call -> common.setPeerIndex ( call -> tries.back ( ).choiceIndex );
	struct TmpStat {
		int frac, jitterMax, jitterAvg;
	};
	TmpStat st [ 4 ];
	std :: memset ( st, 0, sizeof ( st ) );
	for ( int i = 0; i < 4; i ++ )
		if ( int cnt = call -> st [ i ].count ) {
			st [ i ].frac = call -> st [ i ].fracLostSum / cnt;
			st [ i ].jitterMax = call -> st [ i ].jitterMax;
			st [ i ].jitterAvg = call -> st [ i ].jitterSum / cnt;
		}
	if ( ! call -> oked )
		call -> okTime = PTime ( );
	if ( ! call -> cancelled && ! call -> byed )
		call -> endTime = PTime ( );
	std :: time_t timeInSecs = call -> okTime.GetTimeInSeconds ( );
	int secs = conf -> getRounded ( double ( ( call -> endTime - call -> okTime ).GetMilliSeconds ( ) ) / 1000.0 );
	os << "insert into radacctSS values ( 0, '" << call -> from.getUserName ( ) << "', '";
	const CommonCallDetails & common = call -> common;
	bool hasCurChoice = common.curChoice ( );
	if ( hasCurChoice )
		os << common.getCallingDigits ( );
	os << "', '";
	const SourceData & source = common.getSource ( );
	ss :: string outCode, inCode = source.code, inEuCode = source.euCode;
	if ( inEuCode.empty ( ) )
		inEuCode = "-";
	int tarifedSeconds = conf -> getRounded ( conf -> getMinutesFromSeconds ( source.tarif, secs ) * 60 ),
		euTarifedSeconds = conf -> getRounded ( conf -> getMinutesFromSeconds ( source.euTarif, secs ) * 60 );
	int outSecs = 0, inSecs = source.round.round ( tarifedSeconds ),
		inEuSecs = source.euRound.round ( euTarifedSeconds );
	double outPrice = 0, outConnectPrice = 0, inPrice = source.price / 100000.0,
		inConnectPrice = inSecs ? source.connectPrice / 100000.0 : 0,
		inEuPrice = source.euPrice / 100000.0,
		inEuConnectPrice = inEuSecs ? source.euConnectPrice / 100000.0 : 0;
	inConnectPrice += getAmortisedMoney ( source.amortise, tarifedSeconds );
	inEuConnectPrice += getAmortisedMoney ( source.amortise, euTarifedSeconds );
	if ( source.type == SourceData :: card )
		os << "card " << source.acctn << " " << inEuPrice * source.valuteRate << " " << secs <<
			" " << ( inEuSecs ? inEuConnectPrice * source.valuteRate : 0.0 ) << " " <<
			inEuCode << " " << common.getRealDigits ( ) << " " << inEuSecs / 60.0;
	else
		os << common.getDialedDigits ( );
	os << "', '";
	if ( hasCurChoice )
		os << common.getConvertedDigits ( );
	else
		os << "-";
	int ip;
	if ( source.type == SourceData :: inbound && source.peer > 0 )
		ip = source.peer;
	else
		ip = 0;
	os << " " << ip;
	ss :: string outDigits;
	int op = 0;
	if ( hasCurChoice ) {
		outDigits = common.curChoice ( ) -> getRealDigits ( ).substr ( common.curChoice ( ) -> getDepth ( ) );
		op = common.curChoice ( ) -> getPeer ( );
		outCode = common.curChoice ( ) -> getCode ( ).substr ( common.curChoice ( ) -> getDepth ( ) );
		outPrice = common.curChoice ( ) -> getPrice ( ) / 100000.0;
		int outTarifedSeconds = conf -> getRounded ( conf -> getMinutesFromSeconds (
			common.curChoice ( ) -> getTarif ( ), secs ) * 60 );
		outSecs = common.curChoice ( ) -> getRound ( ).round ( outTarifedSeconds );
		outConnectPrice = outSecs ? common.curChoice ( ) -> getConnectPrice ( ) / 100000.0 : 0;
	}
	if ( inCode.empty ( ) )
		inCode = "-";
	if ( outCode.empty ( ) )
		outCode = "-";
	os << " " << op << " " << call -> st [ 2 ].bytesCount << " " << call -> st [ 0 ].bytesCount << " " <<
		call -> codec << " " << inCode << " " << inPrice << " " << inSecs << " " << inConnectPrice << " " <<
		inEuCode << " " << inEuPrice << " " << inEuSecs << " " << inEuConnectPrice << " " << outCode << " " <<
		outPrice << " " << outSecs << " " << outConnectPrice;
	if ( hasCurChoice && ! source.outAcctn.empty ( ) ) {
		const OutCardDetails & ocd = common.curChoice ( ) -> getOutCardDetails ( );
		int ocdTarifedSecs = conf -> getRounded ( conf -> getMinutesFromSeconds ( ocd.getEuTarif ( ), secs ) * 60 );
		int ocdSecs = ocd.getEuRound ( ).round ( ocdTarifedSecs ) ;
		os << " card " << source.outAcctn << " " << ocd.getEuPrice ( ) * source.valuteRate / 100000.0 << " "
			<< secs << " " << ( ocdSecs ? ( ocd.getEuConnectPrice ( ) +
			getAmortisedMoney ( ocd.getAmortise ( ), ocdTarifedSecs ) ) *
			source.valuteRate / 100000.0 : 0.0 ) << " " << ocd.getDigits ( ) << " " << ocd.getEuCode ( )
			<< " " << ocdSecs / 60.0;
	}
	os << "', from_unixtime( " << timeInSecs << " ), from_unixtime( "
		<< call -> endTime.GetTimeInSeconds ( ) << " ), " << secs << ", " << secs << ", '" <<
		common.getCallerIp ( ) << "', '" << call -> myTo.getHostName ( ) << "', " <<
		common.getDisconnectCause ( );
	for ( int i = 0; i < 4; i ++ )
		os << ", " << st [ i ].frac << ", " << st [ i ].jitterMax << ", " << st [ i ].jitterAvg;
	os << ", '" << common.getRealDigits ( ) << "', '" << source.inPrefix << "', '" << outDigits << "', '" <<
		common.getConfId ( ) << "', '" << common.getConfId ( ) << "' )";
	for ( unsigned i = 0; i + 1 < call -> tries.size ( ); i ++ ) {
		int index = call -> tries [ i ].choiceIndex;
		const OutChoiceDetails * curChoice = common.choice ( index );
		os << ", ( 0, '', '";
		if ( curChoice -> getCallingDigits ( ) != "-" )
			os << curChoice -> getCallingDigits ( );
		os << "', '" << curChoice -> getSentDigits ( ) << "', '" << curChoice -> getDigits ( ) << " " << ip << " ";
		op = curChoice -> getPeer ( );
		os << op << " 0 0 - - 0 0 0 - 0 0 0 ";
		ss :: string outCode = curChoice -> getCode ( ).substr ( curChoice -> getDepth ( ) );
		if ( outCode.empty ( ) )
			outCode = '-';
		double outPrice = curChoice -> getPrice ( ) / 100000.0,
			outConnectPrice = 0;//curChoice -> getConnectPrice ( ) / 100000.0;
		os << outCode << " " << outPrice << " 0 " << outConnectPrice << "', from_unixtime( " << timeInSecs <<
			" ), from_unixtime( " << timeInSecs << " ), 0, 0, '', '" <<
			curChoice -> getIp ( ) << "', " << call -> tries [ i ].cause <<
			", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '" << common.getRealDigits ( ) << "', '', '" <<
			common.getRealDigits ( ).substr ( common.choice ( index ) -> getDepth ( ) ) << "', '', '" <<
			common.getConfId ( ) << "' )";
	}

    PSYSTEMLOG(Info, "SipSignallingThread :: accountCall. :/n" << os.str());

	sqlOut -> add ( os.str ( ) );
	deleteCSeq(call->getCallId());
	common.release ( );

	ss :: ostringstream os2;
	os2 << "delete from ActiveCalls where id = " << call -> id;
	sqlOut -> add ( os2.str ( ) );
	conf -> removeActiveCall ( call -> id );
	sendRadiusAcc ( common, call -> st, call -> startTime, call -> inviteTime,
		call -> okTime, call -> endTime, call -> id );
}

bool SipSignallingThread :: receiveMesg ( ) {
// Task: to receive a SIP message
	bool retval = false;
	SIP :: PDU mesg;

	IxcUDPSocket * socket = callerSocket;
	if ( ! readMsg ( socket, mesg ) ) {
		PSYSTEMLOG ( Error, "unable to receive SIP message" );
		return false;
	}
	PSYSTEMLOG ( Info, "New sip message : " << mesg );

	PIPSocket::Address  PeerAddr;
	WORD PeerPort;
	socket -> GetLastReceiveAddress ( PeerAddr, PeerPort );
	myIP = :: getLocalAddress();
	Log -> LogSIPMsg ( mesg, OpengateLog :: Receiving, PeerAddr, PeerPort );
	if ( hasReceiver ( mesg ) )
		//return true;
		retval = true;
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mInvite )
		/*return*/ retval = handleInvite ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mRegister )
		/*return */retval = handleRegister ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mCancel )
		/*return*/retval =  handleCancel ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mAck )
		/*return*/retval = handleAck ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mBye )
		/*return*/retval = handleBye ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mOptions )
		/*return*/ retval = handleOptions ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mRefer )
		/*return*/ retval = handleRefer ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mMessage )
		/*return*/ retval = handleMessage ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mNotify )
		/*return*/ retval = handleNotify ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mSubscribe )
		/*return*/ retval = handleSubcribe ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mInfo )
		/*return*/ retval = handleInfo ( mesg );
/*	else if( mesg.getMethod ( ) == SIP :: PDU :: mUnAuthorized )
		retval = handleUnAuthorized( mesg );
*/
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scInformationTrying )
		/*return*/ retval = handleTrying ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scSuccessfulOK )
		/*return*/ retval = handleOK ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scInformationRinging )
		/*return*/ retval = handleRinging ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scInformationSessionProgress )
		/*return*/ retval = handleRinging ( mesg );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scInformationDialogEstablishement )
		/*return*/ retval = handleTrying ( mesg ); //voobshe neizvestnie nado obrabativat kak 183, no shas ne do etogo
	else if ( ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scFailureUnAuthorised ) ||
		 ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) == SIP :: PDU :: scFailureProxyAuthRequired ) )
		/*return*/ retval = handleUnAuthorised ( mesg, mesg.getStatusCode ( ) );
	else if ( mesg.getMethod ( ) == SIP :: PDU :: mNum && mesg.getStatusCode ( ) > SIP :: PDU :: scSuccessfulOK )
	{
        PSYSTEMLOG(Info, "receiveMesg. befor handleError. " << mesg );
        /*return*/ retval = handleError ( mesg );
         PSYSTEMLOG(Info, "receiveMesg. handleError. " << retval );
         retval |= sendAck ( mesg );
         PSYSTEMLOG(Info, "receiveMesg. sendAck: " << retval );
    }

	PSYSTEMLOG(Info, "Exit from SipSignallingThread :: receiveMesg. Message: " << mesg.getMethod ( ) << "; Status = " << mesg.getStatusCode ( ) );
	return retval;
}

void SipSignallingThread :: checkStalled ( ) {
	PTime now;
	CallsMap :: iterator it = calls.begin ( );
	while ( it != calls.end ( ) ) {
		SipCallDetails * call = it -> second;
		if ( ( call -> cancelled || call -> byed ) ) {
			if ( ( now - call -> endTime ).GetSeconds ( ) > 20 ) {
				PSYSTEMLOG ( Error, "cancelled or byed call" );
				closeThisChoice ( call );
				accountCall ( call );
				CallsMap :: iterator t = it;
				it ++;
				calls.erase ( t );
				continue;
			}
		} else if ( call -> errored && ( now - call -> errorTime ).GetSeconds ( ) > 3 ) {
			PSYSTEMLOG ( Error, "errored call" );
			if ( call -> oked )
				conf -> removeCall ( call );
			accountCall ( call );
			CallsMap :: iterator t = it;
			it ++;
			calls.erase ( t );
			continue;
		} else if ( ! call -> answered ) {
			if ( ( now - call -> inviteTime ).GetSeconds ( ) > 6 && ! tryNextChoice ( call ) ) {
				PSYSTEMLOG ( Error, "not answered call" );
				sendErrBack ( call );
				CallsMap :: iterator t = it;
				it ++;
				accountCall ( call );
				calls.erase ( t );
				continue;
			}
		} else if ( ! call -> goodCall ) {
			if ( ( now - call -> inviteTime ).GetSeconds ( ) > 9 && ( sendByeForward ( call ), ! tryNextChoice ( call ) ) ) {
				PSYSTEMLOG ( Error, "no more choices" );
				sendErrBack ( call );
				CallsMap :: iterator t = it;
				it ++;
				accountCall ( call );
				calls.erase ( t );
				continue;
			}
		} else if ( ! call -> oked ) {
			if ( ( now - call -> inviteTime ).GetSeconds ( ) > 32 ) {
				PSYSTEMLOG ( Error, "not oked call" );
				sendErrBack ( call );
				//sendCancelForward ( call );
				sendCancelForward1 ( call );
				closeThisChoice ( call );
				accountCall ( call );
				CallsMap :: iterator t = it;
				it ++;
				calls.erase ( t );
				continue;
			}
		} else if ( call -> common.getSigOptions ( ).rtpTimeoutEnable ( ) &&
			( now - call -> okTime ).GetSeconds ( ) > 30 &&
			( now - call -> lastTimeoutCheck ).GetSeconds ( ) >
			call -> common.getSigOptions ( ).rtpTimeoutPeriod ( ) ) {
			call -> lastTimeoutCheck = now;
			int i, o;
			call -> session -> getTimeout ( i, o );
			if ( std :: max ( i, o ) > call -> common.getSigOptions ( ).rtpTimeoutOne ( ) ||
				std :: min ( i, o ) > call -> common.getSigOptions ( ).rtpTimeoutBoth ( ) ) {
				PSYSTEMLOG ( Error, "stalled call" );
				sendByeForward ( call );
				sendByeBackward ( call );
				conf -> removeCall ( call );
				closeThisChoice ( call );
				accountCall ( call );
				CallsMap :: iterator t = it;
				it ++;
				calls.erase ( t );
				continue;
			}
		}
		it ++;
	}
}

static bool availableFromCaller ( int s ) {
	return s == - 1 || s == - 3;
}

//static bool availableFromCalled ( int s ) {
//  return s == - 2 || s == - 3;
//}

void SipSignallingThread :: doBalance ( ) {
	StringSet callsToRemove;
	conf -> balanceSipCards ( callsToRemove );
	for ( StringSet :: const_iterator i = callsToRemove.begin ( ); i != callsToRemove.end ( ); i ++ ) {
		CallsMap :: iterator it = calls.find ( * i );
		if ( it == calls.end ( ) )
			continue;
		SipCallDetails * call = it -> second;
		if ( call -> byed )
			continue;
		deleteCSeq(call->getCallId());
		call -> byed = call -> killed = true;
		call -> endTime = PTime ( );
		sendByeForward ( call );
		sendByeBackward ( call );
		conf -> removeCall ( call );
	}
}

void SipSignallingThread :: Main ( ) {
	PSYSTEMLOG ( Info, "SipSignallingThread::Main()" );
	PTime lastStalled;



	while ( callerSocket -> IsOpen ( ) ) {
		int Selection = PSocket :: Select ( * callerSocket, * callerSocket, 1000 );
		if ( Selection > 0 ) {
			PSYSTEMLOG ( Error, "select: " << PChannel :: GetErrorText (
				static_cast < PChannel :: Errors > ( Selection ) ) );
			break;
		}
		if ( availableFromCaller ( Selection ) && ! receiveMesg ( ) ) {
//			PSYSTEMLOG(Info, "receiveMesg ( ) was returned FALSE");
		}
		if ( ( PTime ( ) - lastStalled ).GetSeconds ( ) > 10 ) {
			lastStalled = PTime ( );
			checkStalled ( );
		}
		checkMessages ( );
		doBalance ( );
	}

	calls.clear ( );
	PSYSTEMLOG ( Info, "SipSignallingThread::Main() - end" );

	stopInviteNotifier();
}

void SipSignallingThread :: checkMessages ( ) {
	AsyncMessage m;
	while ( q.getNoWait ( m ) )
		m ( );
}

bool SipSignallingThread :: hasReceiver ( const SIP :: PDU & mesg ) {
	AutoMutex am ( mut );
	ReceiversMap :: iterator i = receivers.find ( mesg.getMIME ( ).getCallID ( ) );
	if ( i == receivers.end ( ) )
		return false;
	PSYSTEMLOG ( Info, "putting message to receiver " << i -> first );
	i -> second -> put ( mesg );
	return true;
}

bool SipSignallingThread :: sendInviteForward ( SipCallDetails * call, const ss :: string & authLine, int authType ) {
    CommonCallDetails & common = call -> common;
	call -> answered = false;
	call -> inviteTime = PTime ( );
	SIP :: PDU & mesg = call -> inviteTemp;
	call -> to = call -> to.shortForm ( );
	SIP :: URL to = call -> to;

	to.setHostName ( common.curChoice ( ) -> getIp ( ) );
	to.setPort ( short ( common.curChoice ( ) -> getPort ( ) ) );
	to.setDisplayName ( "" );
    SIP :: URL to1 = to;

    if(isAccSend_)
    {
        to.setUserName(call -> termCardName.getUserName() );
    }
    else
    {
        to.setUserName ( common.getSentDigits ( ) );
    }
    PSYSTEMLOG(Info, "SipSignallingThread :: sendInviteForward: to: " << to.str() << "; " <<  call -> to.str());
	call -> myTo = to;
	call -> termContact = to.str ( );
	if ( common.getCallingDigits ( ).empty ( ) )
	{
        call -> myFrom.setUserName ( call -> from.getUserName ( ) );
    }
	else if ( common.getCallingDigits ( ) == "-" )
	{
        call -> myFrom.setUserName ( "" );
    }
	else
	{
        if ( false == call -> isAnonymous_ )
            call -> myFrom.setUserName ( common.getCallingDigits ( ) );
    }

	SIP :: MIMEInfo & mime = mesg.getMIME ( );

    ss :: string gkIp = call -> common.curChoice ( ) -> getIp ( );
	ss :: string gkLogin, gkPswd, gkCallId;
	int gkPort;
	if ( conf -> getGkInfo ( gkIp, gkLogin, gkPswd, gkPort ) ) {
		call -> myFrom.setUserName ( gkLogin );
		call -> myFrom.setHostName ( gkIp );
		call -> myFrom.setPort ( short ( gkPort ) );
	}
	setPAssertId(mesg.getMIME ( ).getCallID ( ), createPAssertId(call -> myFrom));
	insertPAssertId(mesg);

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
	call -> origCSeq = mime.getCSeq ( );
	if ( authLine != "" ) {
		ss :: istringstream is ( mime.getCSeq ( ) );
		int cseqNum;
		is >> cseqNum;
		cseqNum ++;
		ss :: ostringstream os;
		os << cseqNum << " INVITE";
		mime.setCSeq ( os.str ( ) );
	}
	SIP :: SessionDescription & sdp = mesg.getSDP ( );

	if(false == handleFastStart ( call, sdp ))
	{
//		sendCancelForward1(call);
		return false;
	}
	setToTerm ( mesg, call, true );
//	insertPAssertId(mesg);
	mesg.getMIME().setRoute("");
	if ( isRecordRouteRequired ( call ) ) {
		mesg.getMIME().setRecordRouteRequired(true);
		mesg.getMIME().clearRecordRoute();
		mesg.getMIME().addRecordRoute(getLocalAddress());
	}

	if ( SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) ) {
		if ( ! common.getTelephoneEventsPayloadType ( ) &&
			common.getSigOptions ( ).isForceInsertingDTMFintoInvite ( ) )
			common.setTelephoneEventsPayloadType ( allocTelephoneEventsPayload ( sdp.getMediaDescriptions ( ) ) );
		if ( unsigned payload = common.getTelephoneEventsPayloadType ( ) ) {
			media -> setTelephoneEventsPayloadType ( payload );

			SIP :: MediaFormat formt ( payload, "telephone-event" );
			formt.setFMTP ( "0-15" );
			media -> addMediaFormat ( formt );
			call -> session -> setTelephoneEventsPayloadType ( payload );
		}
	}
	return sendMessage ( mesg, PIPSocket :: Address ( /*call -> myTo*/to1.getHostName ( ).c_str ( ) ), /*call -> myTo*/to1.getPort ( ) );
}

///////////////////////////////////////////////////////////
// Function returns interface address from the callerSocket
void SipSignallingThread::GetInterfaceAddress(PIPSocket::Address& addr)
{
	callerSocket->GetInterfaceAddress(addr);
}

bool SipSignallingThread :: createResponseOK(SipCallDetails * call, SIP :: PDU & mesg)
{
	SIP :: PDU mesgOK;
	call -> endTime = PTime ( );
	clearContent ( mesgOK );
	mesgOK.setMethod ( SIP :: PDU :: mInvite );

	insertPAssertId(mesgOK);

	setToOrig ( mesgOK, call, false );

	mesgOK.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesgOK.setInfo ( "OK" );

	SIP :: MIMEInfo& mime = mesg.getMIME();

	ss :: string expires = mime.getExpires();
	long newExpires = std :: atol(expires.c_str());
	if(newExpires <= 0 || newExpires > EXPIRES_TIME)
		newExpires = EXPIRES_TIME;

	SIP :: MIMEInfo& mimeOK  = mesgOK.getMIME();
	ss :: ostringstream stringResult;
	stringResult << newExpires;
	mimeOK.setExpires ( stringResult.str ( ) );
//	PSYSTEMLOG(Info, "OK expires = " << newExpires);
//	mime.addRecordRoute(getLocalAddress  ( ));
	mimeOK.setUserAgent ( "PSBC SIP v2.0" );

 	if ( isRecordRouteRequired ( call ) ) {
		mime.setRecordRouteRequired(true);
		StringVector vRecord = mime.getRecordRoute();
		for(unsigned i = 0; i < vRecord.size(); ++i)
		{
			mimeOK.addRecordRoute(vRecord[i], false);
		}

		mimeOK.addRecordRoute(getLocalAddress());
		mesg.getMIME().setRoute("");
	}

	sendMessage ( mesgOK,
				  PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ),
				  call -> myTo.getPort ( ) );

	return true;
}

bool SipSignallingThread :: createTimeOutInvite(SipCallDetails * call)
{
//	SIP :: PDU mesgInvite;
//	clearContent ( mesgInvite );

	call -> endTime = PTime ( );

	SIP :: PDU & mesgInvite = call -> inviteTemp;

	mesgInvite.setMethod ( SIP :: PDU :: mInvite );
	setToTerm ( mesgInvite, call, true );

	mesgInvite.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesgInvite.setInfo ( "Invite" );
	insertPAssertId(mesgInvite);

	SIP :: MIMEInfo& mime = mesgInvite.getMIME();

	ss :: ostringstream stringResult;
	stringResult << inviteTimerInterval_;
	mime.setExpires ( stringResult.str ( ) );
//	PSYSTEMLOG(Info, "InviteTimeOut: expires = " << stringResult);
	mime.setUserAgent ( "PSBC SIP v2.0" );

	call -> origCSeq = mime.getCSeq ( );
	ss :: istringstream is ( mime.getCSeq ( ) );
	int cseqNum;
	is >> cseqNum;
	cseqNum ++;
	ss :: ostringstream os;
	os << cseqNum << " INVITE";
	mime.setCSeq ( os.str ( ) );

///	PSYSTEMLOG(Info, "InviteTimeOut: " << mesgInvite );
	sendMessage ( 	mesgInvite,
					PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ), call -> common.getCallerPort ( ) );
//				  PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ),
//				  call -> myTo.getPort ( ) );

	return true;
}


void SipSignallingThread :: startInviteNotifier ( ) {
	inviteTimer_ = PTimeInterval ( 0, inviteTimerInterval_ );
}

void SipSignallingThread :: stopInviteNotifier()
{
	inviteTimer_.Stop();
}

void SipSignallingThread :: OnInviteTimeout(PTimer &, INT)
{
	PSYSTEMLOG ( Info, "Recieves Invite timer event." );

	CallsMap :: iterator it = calls.find ( inviteTimer_.getCallID ( ) );
	if ( it == calls.end ( ) )
		return;
	SipCallDetails * call = it -> second;

	createTimeOutInvite(call);
}

bool SipSignallingThread :: handleRefer ( SIP :: PDU & mesg )
{
	PSYSTEMLOG(Info, "SipSignallingThread: Refer");
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
		return false;
	SipCallDetails * call = it -> second;
	SIP :: URL tmp = mime.getFrom ( );
	bool fromCaller = call -> from.getUserName ( ) == tmp.getUserName ( ) &&
		call -> from.getHostName ( ) == tmp.getHostName ( );
	PSYSTEMLOG ( Info, "fromCaller: " << fromCaller );
	insertPAssertId(mesg);
	if ( fromCaller ) {
		setToTerm ( mesg, call, true );
		sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
	} else {
		if ( isPreviousHuntFromTerm ( mime, call ) )
			return false;
		call -> calledVia = mime.getVia ( );
		setToOrig ( mesg, call, true );
		mime.setCSeq ( call -> origCSeq );
		sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ),
			call -> common.getCallerPort ( ) );
	}
	return true;
}

bool SipSignallingThread :: handleMessage ( SIP :: PDU & mesg )
{
    ss :: string username = "";
    bool isCard = false;

    // authentication OK
    bool fromNat = true;

    WORD fromPort = 5060;
    PIPSocket ::Address fromAddr;
    if(!handleViaFields(mesg, fromAddr, fromPort))
    {
	    PSYSTEMLOG(Info, "SipSignallingThread: Message. Wrong VIA");
        return false;
    }
{
    SIP :: MIMEInfo & mime = mesg.getMIME ( );

    PString t = fromAddr.AsString ( );
    ss :: string fromIp = static_cast < const char * > ( t );

	if ( conf -> isRegisteredCardAddr ( fromIp, fromPort, username ) ) {
		PSYSTEMLOG ( Info, "MESSAGE: from registered Card " << fromIp << ". Authentication skipped" );
		isCard = true;
	}

    bool needAuthentication = true;
    if ( conf -> isValidInPeerAddress ( fromIp ) ) {
        PSYSTEMLOG ( Info, "(storm): INVITE from InPeer " << fromIp << ". Authentication skipped" );
        needAuthentication = false;
    }
    if(isCard)
    {
        needAuthentication = false;
    }

    if ( needAuthentication ) {
        ss :: string authLine = mime.getAuthorization ( );
        if ( authLine == "" ) {
            sendUnAuthorised ( mesg );
            return false;
		}
		ss :: string password;
		username = sipValidator.getLogin ( authLine );
		IpPortSet addresses;
		addresses.insert ( IpPort ( fromIp, fromPort ) );
		if ( ! conf -> getPassword ( PIPSocket :: Address ( myIP.c_str ( ) ),  addresses, username, password, isCard ) ) {
			PSYSTEMLOG ( Info, "SIPValidator : unknown username " << username );
			sendUnAuthorised ( mesg );
			return false;
		}

		if ( ! sipValidator.isValid ( authLine, "MESSAGE", password, true /*isCNonce(&callDetails)*/ ) ) {
			PSYSTEMLOG ( Info, "SIPValidator : MESSAGE validation failed!" );
			sendUnAuthorised ( mesg );
			return false;
		}

		mime.setAuthorization ( "" );

		if ( conf -> getSipInviteRegistrationPeriod ( ) > 0 ) { // temporary registration for hunting
            IcqContactsVector icqContacts;
            StringSet onlineNumbers, neededNumbers;
            bool h323 = false;
            bool fromNat = true;
            PSYSTEMLOG ( Info, username << " temporary registered on MESSAGE for " << conf -> getSipInviteRegistrationPeriod ( ) << " sec" );
            conf -> registerPeer ( username, fromAddr, fromPort, h323, fromNat,
                PTime ( ) + conf -> getSipInviteRegistrationPeriod ( ) * 1000,
                isCard, fromAddr, fromNat ? fromPort : 5060, neededNumbers,
                onlineNumbers, 0, * ( PUDPSocket * ) 0, icqContacts );
        } else {
            PSYSTEMLOG ( Info, "Temporary registration on MESSAGE for " << username << " disabled" );
        }
    }
}

    SIP :: MIMEInfo & mime = mesg.getMIME ( );
    const SIP :: Content * content = mesg.getContent ( );
    if(!content)
    {
    	PSYSTEMLOG(Error, "Sip message does not contain any data.");
        return false;
    }

    PIPSocket :: Address local;
////////      PIPSocket :: GetHostAddress ( local );
    GetInterfaceAddress(local);
    ss :: string callId = mesg.getMIME ( ).getCallID ( );

    Pointer < SipCallDetails > call;
    call = new SipCallDetails ( local, fromAddr, fromNat, false );
    CommonCallDetails & common = call -> common;
    common.setCallerPort ( fromPort );
    common.setConfId ( callId );

    if ( conf -> isValidInPeerAddress ( common.getCallerIp() ) )
    {
        common.source ( ).type = SourceData :: inbound;
    }
    if ( isCard )
	{
        PSYSTEMLOG ( Info, "(storm): Forced to card " << username );
        common.source ( ).type = SourceData :: card;
        common.source ( ).acctn = username;
    }

	common.setCallingDigitsIn ( call -> from.getUserName ( ) );
	call -> to = mime.getTo ( );
	call -> contact = call -> to;
	call -> contact.setHostName ( getLocalAddress ( ) );
	call -> contact.setPort ( 5060 );


	common.setDialedDigits ( call -> to.getUserName ( ) );

	if( ! conf -> getCallInfo ( call -> choiceForks, call -> forkOutAcctns, common, true, true, true ))
	{
        PSYSTEMLOG(Info, "SipSignallingThread :: handleMessage: conf -> getCallInfo was failed.");

        sendErrMessage ( mesg, common );
        accountCall ( call );
        return false;
	}



    if ( ! mime.updateMaxForwards ( mime )/* || ! conf -> getCallInfo ( call -> choiceForks,
        call -> forkOutAcctns, common, incomingCodecs, true, true ) */) {
        PSYSTEMLOG(Warning, "SipSignallingThread :: handleMessage: updateMaxForwards was failed.");
        sendErrBack ( call );
        accountCall ( call );
        return false;
    }

/**************************/
//	CommonCallDetails & common = call -> common;
	//if ( call -> outTaken )
//	{
//		conf -> release ( common, true );
//		call -> outTaken = false;
//	}
//	PSYSTEMLOG(Info, "common.hasNextPeerIndex: " << common.hasNextPeerIndex());
	while ( common.hasNextPeerIndex ( ) ) {
		common.nextPeerIndex ( );
		if ( ( common.curChoice ( ) -> getIp ( ) == common.getCallerIp ( ) ) &&
			( common.curChoice ( ) -> getPort ( ) == common.getCallerPort ( ) ) ) {

//			PSYSTEMLOG ( Info, "Wrong destination " << common.curChoice ( ) -> getIp () << ":" <<
//				common.curChoice ( ) -> getPort ( ) << ":" << common.getSentDigits ( ) );

			continue;
		}
//		PSYSTEMLOG ( Info, "Trying destination " << common.getCalledIp ( ) << ":" << common.getCalledPort ( )
//			<< ":" << common.getSentDigits ( ) );
		common.sigOptions ( ).setOut ( common.curChoice ( ) -> getSigOptions ( ) );
		int aaa = conf -> getDefaultDisconnectCause(); /*Q931 :: cvNoCircuitChannelAvailable;*/
		if ( ! conf -> take ( common, aaa, 0 ) ) {
			if(common.curChoice())
				PSYSTEMLOG ( Error, "Limit reached: " << common.getCalledIp ( ) );
			continue;
		}
	}

	if(NULL == common.curChoice ( ))
	{
		PSYSTEMLOG(Warning, "SipSignallingThread :: handleMessage: Choice is NULL. Return from handleMessage");
		return false;
	}
/**************************/


//        if(common.curChoice())
//            PSYSTEMLOG(Info, "TO3: Address: " << common.getCalledIp() << ", port: " << common.getCalledPort() );

		PIPSocket :: Address toAddr = PIPSocket :: Address (common.getCalledIp().c_str());
        sendMessage ( mesg, toAddr, common.getCalledPort() );
    {
//        if ( isPreviousHuntFromTerm ( mime, call ) )
//            return false;
//        call -> calledVia = mime.getVia ( );
//        setToOrig ( mesg, call );
//        mime.setCSeq ( call -> origCSeq );
//        sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ), call -> common.getCallerPort ( ) );
    }
	sendOk ( mesg, common, tmMessage );

    return true;

//		sendMessage ( mesg, PIPSocket :: Address ( toAddr.getHostName ( ).c_str ( ) ), toAddr.getPort() );
//	}
//	else
//	{
//		if ( isPreviousHuntFromTerm ( mime, call ) )
//			return false;
//		call -> calledVia = mime.getVia ( );
//		setToOrig ( mesg, call );
//		mime.setCSeq ( call -> origCSeq );
//		sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ), call -> common.getCallerPort ( ) );
//	}
}

bool SipSignallingThread :: handleNotify ( SIP :: PDU & mesg )
{
	PSYSTEMLOG(Info, "SipSignallingThread: Notify");
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
		return false;
	SipCallDetails * call = it -> second;
	SIP :: URL tmp = mime.getFrom ( );
	bool fromCaller = call -> from.getUserName ( ) == tmp.getUserName ( ) &&
		call -> from.getHostName ( ) == tmp.getHostName ( );
	PSYSTEMLOG ( Info, "fromCaller: " << fromCaller );
	insertPAssertId(mesg);
	if ( fromCaller ) {
		setToTerm ( mesg, call, true );
		sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
	} else {
		if ( isPreviousHuntFromTerm ( mime, call ) )
			return false;
		call -> calledVia = mime.getVia ( );
		setToOrig ( mesg, call, true );
		mime.setCSeq ( call -> origCSeq );
		sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ), call -> common.getCallerPort ( ) );
	}
	return true;
}

bool SipSignallingThread :: handleSubcribe ( SIP :: PDU & mesg )
{
	PSYSTEMLOG(Info, "SipSignallingThread: Subscribe");
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
		return false;
	SipCallDetails * call = it -> second;
	SIP :: URL tmp = mime.getFrom ( );
	bool fromCaller = call -> from.getUserName ( ) == tmp.getUserName ( ) && call -> from.getHostName ( ) == tmp.getHostName ( );
	PSYSTEMLOG ( Info, "fromCaller: " << fromCaller );
	if ( fromCaller ) {
		setToTerm ( mesg, call, true );
		sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
	} else {
		if ( isPreviousHuntFromTerm ( mime, call ) )
			return false;
		call -> calledVia = mime.getVia ( );
		setToOrig ( mesg, call, true );
		mime.setCSeq ( call -> origCSeq );
		sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ), call -> common.getCallerPort ( ) );
	}
	return true;
}

bool SipSignallingThread :: handleInfo ( SIP :: PDU & mesg )
{
//	PSYSTEMLOG(Info, "SipSignallingThread: Info");
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	CallsMap :: iterator it = calls.find ( mime.getCallID ( ) );
	if ( it == calls.end ( ) )
		return false;
	SipCallDetails * call = it -> second;
	SIP :: URL tmp = mime.getFrom ( );
	bool fromCaller = call -> from.getUserName ( ) == tmp.getUserName ( ) && call -> from.getHostName ( ) == tmp.getHostName ( );
//	PSYSTEMLOG ( Info, "fromCaller: " << fromCaller );
	if ( fromCaller ) {
		setToTerm ( mesg, call, true );
		sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );
	} else {
		if ( isPreviousHuntFromTerm ( mime, call ) )
			return false;
		call -> calledVia = mime.getVia ( );
		setToOrig ( mesg, call, true );
		mime.setCSeq ( call -> origCSeq );
		sendMessage ( mesg, PIPSocket :: Address ( call -> common.getCallerIp ( ).c_str ( ) ), call -> common.getCallerPort ( ) );
	}
//	sendOk ( mesg, call -> common, tmInfo );
	return true;
}


PInviteTimer::PInviteTimer()
{
}
void PInviteTimer::setCallID(const ss :: string& iCallID)
{/*
	char str[128];
	sprintf(str, "%d", iCallID);
	callID_ = str;
*/
	callID_ = iCallID;
}

ss :: string PInviteTimer::getCallID() const
{
	return callID_;
}

PInviteTimer& PInviteTimer::operator=(const PTimeInterval & time)
{
//	SetInterval(time);
	RunContinuous(time);
	return *this;
}


bool SipSignallingThread :: handleViaFields ( const SIP :: PDU & mesg,
	PIPSocket :: Address & fromAddr, WORD & fromPort ) const {
	bool useVia = conf -> getSupportVia ( );
	callerSocket -> GetLastReceiveAddress ( fromAddr, fromPort );
	PSYSTEMLOG ( Info, "SipSignallingThread::handleViaFields: isSupport = " << useVia );
	if ( ! useVia )
		return true;
	const SIP::MIMEInfo & mime = mesg.getMIME ( );
	ss::string ip;
	if ( ! getIpFromVia ( mime.getVia ( ), ip ) ) {
		PSYSTEMLOG ( Error, "cannot get ip from via" );
		return false;
	}
/*
    /// remove 'rport' from VIA.
    {
        ss :: string viaString = mime.getVia ( );

        ss :: string :: size_type pos = viaString.find ( ";rport" );

        PSYSTEMLOG ( Info, "SipSignallingThread::handleViaFields: befor - viaString = " << viaString );
        if ( pos != ss :: string :: npos )
        {
             viaString =  viaString.substr ( 0, pos );
             PSYSTEMLOG ( Info, "SipSignallingThread::handleViaFields: after - viaString = " << viaString );
        }

        mime.setVia( viaString );
    }
*/
	PIPSocket :: Address viaAddr ( ip.c_str ( ) );
	int viaPort = 0;
	getPortFromVia ( mime.getVia ( ), viaPort );
	if ( viaPort == 0 )
		viaPort = 5060;

	fromAddr = viaAddr;
	fromPort = short ( viaPort );
	PSYSTEMLOG ( Info, "SipSignallingThread::handleViaFields: ip from Via = " << fromAddr << "; port = " <<
		fromPort);
	return true;
}

// recode codecs
void SipSignallingThread :: removeUnsupported ( SipCallDetails * call, SipFastStartElementVector & fs ) const {
	if ( call -> common.getInAllCodecs ( ) )
		return;
	const CodecInfoSet & inAllowedCodecs = call -> common.getInAllowedCodecs ( );
	for ( SipFastStartElementVector :: size_type i = fs.size ( ); i --; ) {
		CodecInfoSet :: const_iterator c = inAllowedCodecs.find ( fs [ i ].codec );
		if ( c == inAllowedCodecs.end ( ) )
			fs.erase ( fs.begin ( ) + i );
		else
			fs [ i ].changeFrames = c -> getFrames ( );
	}
}

void SipSignallingThread :: removeReverseUnsupported ( SipCallDetails * call, SipFastStartElementVector & fs ) const {
	if ( call -> common.getOutAllCodecs ( ) )
		return;
	const CodecInfoSet & outAllowedCodecs = call -> common.getOutAllowedCodecs ( );
	for ( SipFastStartElementVector :: size_type i = fs.size ( ); i --; ) {
		if ( outAllowedCodecs.count ( fs [ i ].codec ) == 0 )
			fs.erase ( fs.begin ( ) + i );
	}
}

void SipSignallingThread :: getNoRecodesMapIn ( SipCallDetails * call, SipFastStartElementVector & fs ) {
	const CodecInfoSet & outAllowedCodecs = call -> common.getOutAllowedCodecs ( );
	for ( unsigned i = 0; i < fs.size ( ); i ++ ) {
		CodecInfoSet :: const_iterator c = outAllowedCodecs.find ( fs [ i ].codec );
		if ( c != outAllowedCodecs.end ( ) && c -> getFrames ( ) )
			fs [ i ].changeFrames = c -> getFrames ( );

		ss :: string codecName = fs [ i ].codec.getCodec ( );
		call -> originalFastStartIn [ codecName ] = fs [ i ];
	}
}

void SipSignallingThread :: getRecodesMapIn ( SipCallDetails * call, SipFastStartElementVector & in ) {
	const CodecInfoSet & outAllowedCodecs = call -> common.getOutAllowedCodecs ( );
	for ( SipFastStartElementVector :: size_type i = in.size ( ); i --; ) {
		SipFastStartElement & e = in [ i ];
		CodecInfoSet :: const_iterator f = findRecode ( e.codec, outAllowedCodecs,
			conf -> getRecodes ( ) );
		if ( f != outAllowedCodecs.end ( ) )
			e.recodeTo = * f;
		if ( e.recodeTo.getCodec ( ) == "unknown" ) {
			in.erase ( in.begin ( ) + i );
			continue;
		}
		e.changeFrames = e.recodeTo.getFrames ( );
		ss :: string codecName = e.recodeTo.getCodec ( );
		call -> originalFastStartIn [ codecName ] = e;
	}
}

void SipSignallingThread :: getNoRecodesMapOut ( SipCallDetails * call, SipFastStartElementVector & fs ) {
	const CodecInfoSet & inAllowedCodecs = call -> common.getInAllowedCodecs ( );
	for ( unsigned i = 0; i < fs.size ( ); i ++ ) {
		CodecInfoSet :: const_iterator c = inAllowedCodecs.find ( fs [ i ].codec );
		if ( c != inAllowedCodecs.end ( ) && c -> getFrames ( ) )
			fs [ i ].changeFrames = c -> getFrames ( );

		ss :: string codec = fs [ i ].codec.getCodec ( );
		call -> originalFastStartOut [ codec ] = fs [ i ];
	}
}

struct SipFastStartTemp {
	SipFastStartElement e;
	int origPref;
	int codecPref;
	SipFastStartTemp ( const SipFastStartElement & ee, int o, int c ) : e ( ee ), origPref ( o ), codecPref ( c ) { }
	SipFastStartTemp ( ) { }
};

void SipSignallingThread :: getRecodesMapOut ( SipCallDetails * call, SipFastStartElementVector & out ) {
	typedef std :: map < ss :: string, SipFastStartTemp, std :: less < ss :: string >,
		__SS_ALLOCATOR < std :: pair < const ss :: string, SipFastStartTemp > > > StringSipFastStartTempMap;
	StringSipFastStartTempMap tmp;

	const CodecInfoSet & inAllowedCodecs = call -> common.getInAllowedCodecs ( );
	const CodecInfoSet & outAllowedCodecs = call -> common.getOutAllowedCodecs ( );
	for ( SipFastStartElementVector :: size_type i = out.size ( ); i --; ) {
		SipFastStartElement e = out [ i ];
		StringStringSetMap :: const_iterator f = conf -> getBackRecodes ( ).find ( e.codec.getCodec ( ) );
		if ( f == conf -> getBackRecodes ( ).end ( ) ) {
			out.erase ( out.begin ( ) + i );
			continue;
		}
		int cp = 0;
		int chfr = 0;
		CodecInfoSet :: const_iterator ic = inAllowedCodecs.find ( e.codec );
		if ( ic != inAllowedCodecs.end ( ) && ic -> getFrames ( ) )
			chfr = ic -> getFrames ( );
		for ( StringSet :: const_iterator r = f -> second.begin ( ); r != f -> second.end ( ); ++ r, cp ++ ) {
			CodecInfo c;
			CodecInfoSet :: const_iterator j = outAllowedCodecs.find ( * r );
			if ( j != outAllowedCodecs.end ( ) )
				c = * j;
			if ( c.getCodec ( ) == "unknown" )
				continue;
			e.recodeTo = c;
			e.changeFrames = chfr;
			e.outChangeFrames = c.getFrames ( );
			tmp [ c.getCodec ( ) ] = SipFastStartTemp ( e, int ( i ), cp );
		}
	}
	typedef std :: map < int, SipFastStartElement, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, SipFastStartElement > > > IntSipFastStartElementMap;
	typedef std :: map < int, IntSipFastStartElementMap, std :: less < int >,
		__SS_ALLOCATOR < std :: pair < const int, IntSipFastStartElementMap > > > IntIntSipFastStartElementMap;

	IntIntSipFastStartElementMap tmp2;
	for ( StringSipFastStartTempMap :: const_iterator i = tmp.begin ( ); i != tmp.end ( ); ++ i ) {
		const SipFastStartTemp & t = i -> second;
		tmp2 [ t.origPref ] [ t.codecPref ] = t.e;
		ss :: string codec = t.e.recodeTo.getCodec ( );
		call -> originalFastStartOut [ codec ] = t.e;
	}
	out.clear ( );
	for ( IntIntSipFastStartElementMap :: const_iterator i = tmp2.begin ( ); i != tmp2.end ( ); ++ i )
		for ( IntSipFastStartElementMap :: const_iterator j = i -> second.begin ( );
			j != i -> second.end ( ); ++ j )
			out.push_back ( j -> second );

}

static bool setCodec ( SIP :: MediaFormat & format, const ss :: string & c, int setFrames ) {
	if ( c == "g729r8" || c == "g729ar8" ) {
		SIP :: MediaFormat f ( SIP :: MediaFormat :: G729, "G729", 8000, "", "annexb=no" );
		if ( setFrames )
			f.setPtime ( setFrames * 10 );
		format = f;
		return true;
	}
	if ( c == "g729abr8" || c == "g729br8" ) {
		SIP :: MediaFormat f ( SIP :: MediaFormat :: G729, "G729" );
		if ( setFrames )
			f.setPtime ( setFrames * 10 );
		format = f;
		return true;
	}
	if ( c == "g723ar" ) {
		SIP :: MediaFormat f ( SIP :: MediaFormat :: G7231, "G723" );
		if ( setFrames )
			f.setPtime ( setFrames * 30 );
		format = f;
		return true;
	}
	if ( c == "g723ar53" ) {
		SIP :: MediaFormat f ( SIP :: MediaFormat :: G7231, "G723", 8000, "", "bitrate=5300" );
		if ( setFrames )
			f.setPtime ( setFrames * 30 );
		format = f;
		return true;
	}
	if ( c == "g723ar63" ) {
		SIP :: MediaFormat f ( SIP :: MediaFormat :: G7231, "G723", 8000, "", "bitrate=6300" );
		if ( setFrames )
			f.setPtime ( setFrames * 30 );
		format = f;
		return true;
	}
	if ( c == "g7231" ) {
		SIP :: MediaFormat f ( SIP :: MediaFormat :: G7231, "G723", 8000, "", "annexa=no" );
		if ( setFrames )
			f.setPtime ( setFrames * 30 );
		format = f;
		return true;
	}
	if ( c == "g726r16" || c == "g726r24" ||
		c == "g726r32" || c == "g726" ) {
		SIP :: MediaFormat f ( SIP :: MediaFormat :: G726, "G726" );
		format = f;
		return true;
	}
	if ( c == "g711alaw" ) {
		SIP :: MediaFormat f ( SIP :: MediaFormat :: PCMA, "PCMA" );
		if ( setFrames )
		{	//f.setPtime ( setFrames * 10 ); // bug #1993
			f.setPtime ( setFrames );
		}
		format = f;
		return true;
	}
	if ( c == "g711ulaw" ) {
		SIP :: MediaFormat f ( SIP :: MediaFormat :: PCMU, "PCMU" );
		if ( setFrames )
		{	//f.setPtime ( setFrames * 10 ); // bug #1993
			f.setPtime ( setFrames );
		}
		format = f;
		return true;
	}
	if ( c == "g728" ) {
		SIP :: MediaFormat f ( SIP :: MediaFormat :: G728, "G728" );
		format = f;
		return true;
	}
	if ( c == "gsmefr" || c == "gsmfr" ) {
		SIP :: MediaFormat f ( SIP :: MediaFormat :: GSM, "GSM" );
		format = f;
		return true;
	}
	PSYSTEMLOG ( Error, "unsupported codec in setCodec: " << c );
	SIP :: MediaFormat f ( SIP :: MediaFormat :: G729, "G729", 8000, "", "annexb=no" );
	if ( setFrames )
		f.setPtime ( setFrames * 10 );
	format = f;
	return false;
}

static void changeDataTypeAndFramesIn ( SipFastStartElement & e ) {
	ss :: string c = e.recodeTo.getCodec ( );
	if ( c != "unknown" )
		setCodec ( e.format, c, e.changeFrames );
	e.codec = :: getCodec ( e.format );
	return;
}

SipFastStartElement :: SipFastStartElement ( const SIP :: MediaFormat & f, int ch ) : format ( f ),
	codec ( getCodec ( f ) ), changeFrames ( ch ) { }

bool SipSignallingThread :: handleFastStart ( SipCallDetails * call, SIP :: SessionDescription & sdp ) {
    PIPSocket :: Address intrfAddr ( INADDR_ANY );
	CommonCallDetails & common = call -> common;
	IntVector :: size_type i_size = common.curChoice ( ) -> getInterfaces ( ).size ( );
	for ( IntVector :: size_type idx = 0; idx < i_size; idx ++ ) {
		if ( common.curChoice ( ) -> getInterfaces ( ) [ idx ] == INADDR_ANY )
			continue;
		intrfAddr = common.curChoice ( ) -> getInterfaces ( ) [ idx ];
		break;
	}
	if( intrfAddr != INADDR_ANY ) {
		ss :: string strIntrfIp ( static_cast < const char * > ( PIPSocket ::Address ( intrfAddr ).AsString ( ) ) );
		call -> termInterface = strIntrfIp;
	} else
		call -> termInterface = myIP;
	call -> myVia = "SIP/2.0/UDP " + call -> getTermInterface ( );
	ss :: string :: size_type pos = call -> via.find ( ';' );
	if ( pos != ss :: string :: npos )
		call -> myVia += call -> via.substr ( pos );
    if ( false == call -> isAnonymous_)
    {
        call -> myFrom.setHostName ( call -> getTermInterface ( ) );
    }

    if(! call -> getDirectRTP ( ))
    {
        sdp.setDefaultConnectAddress ( call -> termInterface );
        sdp.setOwnerAddress ( call -> termInterface );
    }
    else
    {
        sdp.setOwnerAddress ( sdp.getDefaultConnectAddress () );
    }

    SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio );
	if ( ! media )
		return false;
	int localRtpPort = call -> session -> getLocalAddress ( true, false );
    if(! call -> getDirectRTP ( ))
    {
        media -> setTransportAddress ( SIP :: TransportAddress ( call -> termInterface, short ( localRtpPort) ) );
    }
    else
    {
        media -> setTransportAddress ( SIP :: TransportAddress ( sdp.getDefaultConnectAddress ().getAddr(), media ->getTransportAddress().getPort() ) );
    }
	const SIP :: MediaFormatVector & fastStart = media -> getMediaFormats ( );
	SipFastStartElementVector fsin, fsout;
	call -> originalFastStartIn.clear ( );
	call -> originalFastStartOut.clear ( );
	for ( SIP :: MediaFormatVector :: const_iterator i = fastStart.begin ( ); i != fastStart.end ( ); ++ i ) {
		fsin.push_back ( SipFastStartElement ( * i ) );
		fsout.push_back ( SipFastStartElement ( * i ) );
	}
	removeUnsupported ( call, fsin );
	removeUnsupported ( call, fsout );
	if ( fsin.empty ( ) || fsout.empty ( ) )
		return false;
	SipFastStartElementVector tmp = fsin;
	removeReverseUnsupported ( call, tmp );
/*
	{
//		ss :: ostringstream inputCodecs;
		ss :: string inputCodecs;
		for(unsigned i = 0; i < tmp.size(); ++i)
		{
			inputCodecs += tmp[i]. codec.getCodec();
			inputCodecs += "; ";
		}
		PSYSTEMLOG(Info, "fs IN: " << inputCodecs);
//		ss :: ostringstream inputCodecs1;
		ss :: string inputCodecs1;
		const CodecInfoSet & outAllowedCodecs = call -> common.getOutAllowedCodecs ( );
		for(CodecInfoSet :: const_iterator j = outAllowedCodecs.begin(); j != outAllowedCodecs.end(); ++j)
		{
			inputCodecs1 += (*j).getCodec().c_str() ;
			inputCodecs1 += "; ";
		}
		PSYSTEMLOG(Info, "Allow: " << inputCodecs1);
	}
*/
	outputFastStartVectorData("FS In" ,tmp);


	bool direct = call -> getDirectRTP ( );
	if ( tmp.empty ( ) ) {
		if ( ! direct )
			getRecodesMapIn ( call, fsin );
		else
			fsin.swap ( tmp );
	} else {
		fsin.swap ( tmp );
		getNoRecodesMapIn ( call, fsin );
	}
	outputFastStartVectorData("Fsin" ,fsin);
	if ( fsin.empty ( ) )
		return false;
	tmp = fsout;
	removeReverseUnsupported ( call, tmp );
	outputFastStartVectorData("FS out" ,tmp);
	if ( tmp.empty ( ) ) {
		if ( ! direct )
			getRecodesMapOut ( call, fsout );
		else
			fsout.swap ( tmp );
	} else {
		fsout.swap ( tmp );
		getNoRecodesMapOut ( call, fsout );
	}
	outputFastStartVectorData("Fsout" ,fsout);
	if ( fsout.empty ( ) )
		return false;
	SIP :: MediaFormatVector retFs;
	for ( unsigned i = 0; i < fsin.size ( ); i ++ ) {
		changeDataTypeAndFramesIn ( fsin [ i ] );
		retFs.push_back ( fsin [ i ].format );
	}
	media -> setMediaFormats ( retFs );
	return true;
}


/*******************************************************************************/
bool SipSignallingThread :: handleReinviteFromTerm ( SipCallDetails * call, SIP :: PDU & mesg )
{
	PSYSTEMLOG ( Info, "SIP reinvite from term " );
    insertPAssertId(mesg);

    SIP :: MIMEInfo & mime = mesg.getMIME ( );

//    CommonCallDetails & common = call -> common;

    {
/* From Terminator.
        common.setCallingDigitsIn ( call -> from.getUserName ( ) );
        PSYSTEMLOG ( Info, call -> from.str ( ) );
        call -> to = mime.getTo ( );
        call -> contact = call -> to;
        call -> contact.setHostName ( :: getLocalAddress ( ) );
        common.setDialedDigits ( call -> to.getUserName ( ) );
*/
        CodecInfoVector incomingCodecs;
        SIP :: SessionDescription & sdp = mesg.getSDP ( );
        getIncomingCodecsFromSDP ( sdp, incomingCodecs );
        ss :: ostringstream os;

        bool direct = call -> getDirectRTP ( );
        if(true == direct) {
            if ( SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) )
                handleFastStartResponse ( call, media );
        }

//        call -> via = mime.getVia ( );
//        call -> myFrom = call -> from;

        SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio );
        if ( ! media )
            return false;
        media -> getTransportAddress ( ).getIpAndPort ( call -> origRtpAddr, call -> origRtpPort );

//    call -> myVia = "SIP/2.0/UDP " + call -> getTermInterface ( );
//    ss :: string :: size_type pos = call -> via.find ( ';' );
//    if ( pos != ss :: string :: npos )
//        call -> myVia += call -> via.substr ( pos );
//    call -> myFrom.setHostName ( call -> getTermInterface ( ) );
    sdp.setDefaultConnectAddress ( call -> termInterface ); // ?????????
    sdp.setOwnerAddress ( call -> termInterface ); // ???????

    {
        PIPSocket :: Address addr;
        WORD port;
        media -> getTransportAddress ( ).getIpAndPort ( addr, port );
        call -> session -> setSendAddress ( true, false, addr, port, 0, 0, false, "", "" );
        call -> session -> setSendAddress ( true, true, addr, WORD ( port + 1 ), 0, 0, false, "", "" );
        port = WORD ( call -> session -> getLocalAddress ( true, false ) );
        media -> setTransportAddress ( SIP :: TransportAddress ( myIP, port ) );
        PSYSTEMLOG(Info, "3 Exit REINVITE: message: " << *media << ", Addr=" << myIP << ",Port=" << port);
    }

    const SIP :: MediaFormatVector & fastStart = media -> getMediaFormats ( );
    SipFastStartElementVector fsin, fsout;
    call -> originalFastStartIn.clear ( );
    call -> originalFastStartOut.clear ( );
    for ( SIP :: MediaFormatVector :: const_iterator i = fastStart.begin ( ); i != fastStart.end ( ); ++ i ) {
        fsin.push_back ( SipFastStartElement ( * i ) );
        fsout.push_back ( SipFastStartElement ( * i ) );
    }
    removeUnsupported ( call, fsin );
    removeUnsupported ( call, fsout );
    if ( fsin.empty ( ) || fsout.empty ( ) )
    {
        PSYSTEMLOG(Info, "Reinvite: fsin.size()=" << fsin.size() << ", fsout.size( )=" << fsout.size( ));
        return false;
    }
    SipFastStartElementVector tmp = fsin;
    outputFastStartVectorData("FS In" ,tmp);
    removeReverseUnsupported ( call, tmp );
//	bool direct = call -> getDirectRTP ( );
    if ( tmp.empty ( ) ) {
        if ( ! direct )
            getRecodesMapIn ( call, fsin );
        else
            fsin.swap ( tmp );
    } else {
        fsin.swap ( tmp );
        getNoRecodesMapIn ( call, fsin );
    }
    if ( fsin.empty ( ) )
    {
        PSYSTEMLOG(Info, "fsin.empty ( )");
        return false;
    }

    tmp = fsout;
    outputFastStartVectorData("FS Out" ,tmp);
    removeReverseUnsupported ( call, tmp );
    if ( tmp.empty ( ) ) {
        if ( ! direct )
            getRecodesMapOut ( call, fsout );
        else
            fsout.swap ( tmp );
    } else {
        fsout.swap ( tmp );
        getNoRecodesMapOut ( call, fsout );
    }
    if ( fsout.empty ( ) )
    {
        PSYSTEMLOG(Info, "fsout.empty ( )");
        return false;
    }
    SIP :: MediaFormatVector retFs;
    for ( unsigned i = 0; i < fsin.size ( ); i ++ ) {
        changeDataTypeAndFramesIn ( fsin [ i ] );
        retFs.push_back ( fsin [ i ].format );
    }
    media -> setMediaFormats ( retFs );
    call -> origCSeq = mime.getCSeq ( );

/*******************************************************************************/

//?????         setToOrig ( mesg, call );
//		PSYSTEMLOG(Info, "3 Exit REINVITE: message: " << mesg << "Addr=" << call -> origRtpAddr << ",Port=" << call -> origRtpPort);

// ?????        sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );

    }
    return true;
}

bool SipSignallingThread :: handleReinviteFromOrig ( SipCallDetails * call, SIP :: PDU & mesg )
{
	PSYSTEMLOG ( Info, "SIP reinvite" );

	insertPAssertId(mesg);

	SIP :: MIMEInfo & mime = mesg.getMIME ( );

	CommonCallDetails & common = call -> common;
//	if(mesg.hasSDP())
	{
		common.setCallingDigitsIn ( call -> from.getUserName ( ) );
		PSYSTEMLOG ( Info, call -> from.str ( ) );
		call -> to = mime.getTo ( );
		call -> contact = call -> to;
		call -> contact.setHostName ( :: getLocalAddress ( ) );
		common.setDialedDigits ( call -> to.getUserName ( ) );
//		SourceData & source = common.source ( );
		CodecInfoVector incomingCodecs;
		SIP :: SessionDescription & sdp = mesg.getSDP ( );
		getIncomingCodecsFromSDP ( sdp, incomingCodecs );
		ss :: ostringstream os;

		bool direct = call -> getDirectRTP ( );
		if(true == direct) {
			if ( SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) )
				handleFastStartResponse ( call, media );
		}

		call -> via = mime.getVia ( );
		call -> myFrom = call -> from;

		if ( SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) )
		{
			media -> getTransportAddress ( ).getIpAndPort ( call -> origRtpAddr, call -> origRtpPort );
		}
		call -> inviteTemp = mesg;
/*******************************************************************************/
//		handleFastStart ( call, sdp );

	SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio );
	if ( ! media )
		return false;
   call -> myVia = "SIP/2.0/UDP " + call -> getTermInterface ( );
	ss :: string :: size_type pos = call -> via.find ( ';' );
	if ( pos != ss :: string :: npos )
		call -> myVia += call -> via.substr ( pos );
	call -> myFrom.setHostName ( call -> getTermInterface ( ) );
	sdp.setDefaultConnectAddress ( call -> termInterface );
	sdp.setOwnerAddress ( call -> termInterface );
//	int localRtpPort = call -> session -> getLocalAddress ( true, false );
//	media -> setTransportAddress ( SIP :: TransportAddress ( call -> termInterface, localRtpPort) );
//	media -> setTransportAddress ( SIP :: TransportAddress ( call -> origRtpAddr, call -> origRtpPort) );
	{
			PIPSocket :: Address addr;
			WORD port;
			media -> getTransportAddress ( ).getIpAndPort ( addr, port );
			call -> session -> setSendAddress ( true, false, addr, port, 0, 0, false, "", "" );
			call -> session -> setSendAddress ( true, true, addr, WORD ( port + 1 ), 0, 0, false, "", "" );
			port = WORD ( call -> session -> getLocalAddress ( true, false ) );
			media -> setTransportAddress ( SIP :: TransportAddress ( myIP, port ) );
		PSYSTEMLOG(Info, "3 Exit REINVITE orig: message: " << *media << ", Addr=" << myIP << ",Port=" << port);
	}

	const SIP :: MediaFormatVector & fastStart = media -> getMediaFormats ( );
	SipFastStartElementVector fsin, fsout;
	call -> originalFastStartIn.clear ( );
	call -> originalFastStartOut.clear ( );
	for ( SIP :: MediaFormatVector :: const_iterator i = fastStart.begin ( ); i != fastStart.end ( ); ++ i ) {
		fsin.push_back ( SipFastStartElement ( * i ) );
		fsout.push_back ( SipFastStartElement ( * i ) );
	}
	removeUnsupported ( call, fsin );
	removeUnsupported ( call, fsout );
	if ( fsin.empty ( ) || fsout.empty ( ) )
	{
		PSYSTEMLOG(Info, "Reinvite: fsin.size()=" << fsin.size() << ", fsout.size( )=" << fsout.size( ));
		return false;
	}
	SipFastStartElementVector tmp = fsin;
	outputFastStartVectorData("FS In" ,tmp);
	removeReverseUnsupported ( call, tmp );
//	bool direct = call -> getDirectRTP ( );
	if ( tmp.empty ( ) ) {
		if ( ! direct )
			getRecodesMapIn ( call, fsin );
		else
			fsin.swap ( tmp );
	} else {
		fsin.swap ( tmp );
		getNoRecodesMapIn ( call, fsin );
	}
	if ( fsin.empty ( ) )
	{
		PSYSTEMLOG(Info, "fsin.empty ( )");
		return false;
	}

	tmp = fsout;
	outputFastStartVectorData("FS Out" ,tmp);
	removeReverseUnsupported ( call, tmp );
	if ( tmp.empty ( ) ) {
		if ( ! direct )
			getRecodesMapOut ( call, fsout );
		else
			fsout.swap ( tmp );
	} else {
		fsout.swap ( tmp );
		getNoRecodesMapOut ( call, fsout );
	}
	if ( fsout.empty ( ) )
	{
		PSYSTEMLOG(Info, "fsout.empty ( )");
	 	return false;
	}
	SIP :: MediaFormatVector retFs;
	for ( unsigned i = 0; i < fsin.size ( ); i ++ ) {
		changeDataTypeAndFramesIn ( fsin [ i ] );
		retFs.push_back ( fsin [ i ].format );
	}
	media -> setMediaFormats ( retFs );
	call -> origCSeq = mime.getCSeq ( );

/*******************************************************************************/

		setToTerm ( mesg, call, true );
//		PSYSTEMLOG(Info, "3 Exit REINVITE: message: " << mesg << "Addr=" << call -> origRtpAddr << ",Port=" << call -> origRtpPort);

		sendMessage ( mesg, PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( ) );

	}
	return true;
}

ss :: string SipSignallingThread :: createPAssertId(const SIP::URL & from1)
{
	ss :: string from = from1.str();

	ss :: string :: size_type startIndex = from.find ( '\"' );
	ss :: string :: size_type endIndex = from.find( '\"', startIndex+1);
	//int endIndex2 = from.find(">");
	ss :: string :: size_type size(0);
	if(startIndex != ss :: string :: npos && endIndex != ss :: string :: npos)
	{
		size = endIndex - startIndex - 1;
	}
	ss :: string name(from.data() + (startIndex + 1), size);

	ss :: string pAssertID("\"");
	if(true == name.empty())
		name = from1.getUserName();

	pAssertID += name;
	pAssertID += "\" <sip:+";
	pAssertID += from1.getUserName();
	pAssertID += '@';
	pAssertID += from1.getHostName();
	pAssertID += '>';
//PSYSTEMLOG(Info, "2. createPAssertId: " <<  pAssertID);
	return pAssertID;
}

bool SipSignallingThread :: isRecordRouteRequired ( const SIP :: PDU & mesg ) {
	ss :: string callId = mesg.getMIME ( ).getCallID ( );
	CallsMap :: iterator it = calls.find ( callId );
	if ( it == calls.end ( ) )
		return false;
	return isRecordRouteRequired ( it -> second );
}

bool SipSignallingThread :: isRecordRouteRequired ( const SipCallDetails * call ) {
	return call -> common.getSigOptions ( ).isRecordRouteRequired ( );
}

bool SipSignallingThread :: isCNonceTerm(CommonCallDetails* call) const
{
	bool isCNonce = false;
	if(NULL == call)
		return false;

	const OutChoiceDetails * cur = call->curChoice ( );
	if(cur)
	{
		isCNonce = cur->getSigOptions ( ).isUseCNonce();
		PSYSTEMLOG(Info, "Term. isCNonce: from choices " << isCNonce);
	}
	else
	{
		PSYSTEMLOG(Info, "Term. isCNonce: choices Is Null");
	}

	return isCNonce;
}

bool SipSignallingThread :: isCNonceOrig(CommonCallDetails* call) const
{
	bool isCNonce = false;
	if(NULL == call)
		return false;
	const OutChoiceDetails * cur = call->curChoice ( );
	if(cur)
	{
		isCNonce = cur->getSigOptions ( ).isUseCNonce();
		PSYSTEMLOG(Info, "Orig. isCNonce: from choices " << isCNonce);
	}
	else
	{
		PSYSTEMLOG(Info, "Orig isCNonce: choices Is Null");
	}

	return isCNonce;
}

bool SipSignallingThread :: isCNonce(CommonCallDetails* call) const
{
	return true;
	return isCNonceOrig(call) || isCNonceTerm(call);
}

void SipSignallingThread :: insertPAssertId(SIP :: PDU & mesg)
{
	SIP::MIMEInfo & mime = mesg.getMIME ( );
	ss :: string callId = mesg.getMIME ( ).getCallID ( );

	CallsMap :: iterator it = calls.find ( callId );
	Pointer < SipCallDetails > call;
	if(it != calls.end())
	{
		call = it -> second;
		CommonCallDetails & common = call -> common;
		const OutChoiceDetails * cur = common.curChoice ( );
		if (cur)
		{
			bool pAssertReq = cur->getSigOptions ( ).isPAssertIdRequired();
			bool isRecordRouteRequired = cur->getSigOptions ( ).isRecordRouteRequired();
			PSYSTEMLOG(Info, "insertPAssertId: " <<  pAssertReq << "; isRecordRouteRequired = " << isRecordRouteRequired);

			if (true == pAssertReq)
			{
				ss :: string pAssertID = getPAssertId(mime.getCallID ( ));
				PSYSTEMLOG(Info, "insertPAssertId: " <<  pAssertID << " will be inserted into MIME. CallId = " <<  mime.getCallID ( ));
				mime.setPAssertID(pAssertID);
			}
		}
	}
}

void SipSignallingThread :: deletePAssertId(SIP :: PDU & mesg)
{
	SIP::MIMEInfo & mime = mesg.getMIME ( );
	ss :: string callId = mesg.getMIME ( ).getCallID ( );

	CallsMap :: iterator it = calls.find ( callId );
	Pointer < SipCallDetails > call;
	if(it != calls.end())
	{
		call = it -> second;
		CommonCallDetails & common = call -> common;
		const OutChoiceDetails * cur = common.curChoice ( );
		if(cur)
		{
			bool pAssertReq = cur->getSigOptions ( ).isPAssertIdRequired();
			PSYSTEMLOG(Info, "deletePAssertId: " <<  pAssertReq);

			if(false == pAssertReq)
			{
				ss :: string pAssertID = getPAssertId(mime.getCallID ( ));
				PSYSTEMLOG(Info, "deletePAssertId: " <<  pAssertID << " will be removed from MIME. CallId = " <<  mime.getCallID ( ));
				mime.removePAssertID();
			}
		}
	}
}

void SipSignallingThread :: setPAssertId(const ss :: string& callId, const ss :: string& p_asserted_identity)
{
//	PSYSTEMLOG(Info, "setPAssertId: " << p_asserted_identity);
	MapOfSequences::iterator it = mapOfSequences_.find(callId);
	if(it == mapOfSequences_.end())
	{
		CallInternalData cid;
		cid.p_asserted_identity_ = p_asserted_identity;
		mapOfSequences_.insert(make_pair(callId, cid));
//		PSYSTEMLOG(Info, "setPAssertId: new ");
	}
	else
	{
		if(it->second.p_asserted_identity_.empty())
		{
			it->second.p_asserted_identity_ = p_asserted_identity;
//			PSYSTEMLOG(Info, "setPAssertId: empty");
		}
	}
}

ss :: string SipSignallingThread :: getPAssertId(const ss :: string& callId) const
{
	MapOfSequences::const_iterator it = mapOfSequences_.find(callId);
	if(it != mapOfSequences_.end())
	{
		return it->second.p_asserted_identity_;
	}

	return ss :: string();
}

ss :: string SipSignallingThread :: parsesAdditionalNumber ( const ss :: string & strFrom,
	ss :: string * /*newNumber*/, ss :: string * addNumber ) {
	ss :: string newString;

	ss :: string :: size_type iBegNum = strFrom.find ( "<" );
    if(iBegNum == ss :: string :: npos )
        iBegNum = 0;
	ss :: string :: size_type iBegin = strFrom.find ( "*", iBegNum );
	if ( iBegin == ss :: string :: npos )
	{ // Esli v nomere otsytstvuet '*'(*67), to proveryaem na nalichie slova "Anonymous".
        ss :: string :: size_type isAnonymous = strFrom.find ( "Anonymous", iBegNum );
        if ( isAnonymous == ss :: string :: npos )
        {
            if(addNumber)
            {
                *addNumber = "Anonymous";
            }
        }
        return strFrom;
    }
	ss :: string :: size_type iAT = strFrom.find ( "@" );

	* addNumber = strFrom.substr ( iBegin, 3 );
	newString = strFrom.substr ( iBegin + 3, iAT - ( iBegin + 3 ) );
	return editNumberTo ( strFrom, newString );
}

ss :: string SipSignallingThread :: editNumberTo ( const ss :: string & oldFullNumber,
	const ss :: string & newNumber ) {
	ss :: string newFullNumber;
	ss :: string :: size_type iBegNum = oldFullNumber.find ( ":" );
	ss :: string :: size_type iAT = oldFullNumber.find ( "@" );

	newFullNumber = oldFullNumber.substr ( 0, iBegNum + 1 );
	newFullNumber += newNumber;
	newFullNumber += oldFullNumber.substr ( iAT, oldFullNumber.size ( ) - iAT );

	return newFullNumber;
}

SipSignallingThread :: AdditionalNumber SipSignallingThread :: choiceAdditionalData ( const ss :: string & addNumber) {
	AdditionalNumber an = anNone;
	if ( addNumber == "*67" )
		an = SipSignallingThread :: anAnonymous;
    else if ( addNumber == "Anonymous" )
        an = SipSignallingThread :: anAnonymous1;

	return an;
}

ss :: string SipSignallingThread :: editNumberFromAnonymous ( ) const {
	return "\"Anonymous\" <sip:Anonymous@Anonymous.invalid>";
}

void SipSignallingThread :: sendOk ( const SIP :: PDU& mesg, const CommonCallDetails & common, enTypeMessage tm )
{
//	PSYSTEMLOG(Info, "SipSignallingThread :: sendOk");
	SIP :: PDU mesgOK;
	mesgOK.setStatusCode ( SIP :: PDU :: scSuccessfulOK );
	mesgOK.setInfo ( "OK" );
	SIP :: MIMEInfo & mimeOK = mesgOK.getMIME ( );
	const SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mimeOK.setVia ( mime.getVia ( ) );
	mimeOK.setFrom ( mime.getFrom ( ) );
	mimeOK.setTo ( mime.getTo ( ) );
	mimeOK.setCallID ( mime.getCallID ( ) );
	mimeOK.setContact ( mime.getContact ( ) ); // .bracketShortForm ( )

/// get CSeq index:
	if ( tmMessage == tm ) {
		char pstr [ 255 ];
		sprintf ( pstr, "%d MESSAGE", mime.getCSeqIndex ( ) );
		mimeOK.setCSeq ( pstr );
	}
	if ( tmInfo == tm ) {
		char pstr [ 255 ];
		sprintf ( pstr, "%d INFO", mime.getCSeqIndex ( ) );
		mimeOK.setCSeq ( pstr );
	}

	mimeOK.setAccept ( "application/sdp" );
	mimeOK.setMaxForwards ( 70 );
	mimeOK.setUserAgent ( "PSBC SIP v2.0" );
/*
	mimeOK.setRecordRouteRequired(isRecordRouteRequired());
	if(isRecordRouteRequired())
	{
		for(unsigned i = 0; i < vRecordRoute.size(); ++i)
		{
		    mimeOK.addRecordRoute(vRecordRoute[i], false);
		}
		mimeOK.addRecordRoute(localIp);
	}
*/
//	mime.addRecordRoute(common.getCallerIp ( ));
//	mime.clearRecordRoute();

	sendMessage ( mesgOK, PIPSocket :: Address ( common.getCallerIp ( ).c_str() ), common.getCallerPort ( ) );
//	PIPSocket :: Address ( call -> myTo.getHostName ( ).c_str ( ) ), call -> myTo.getPort ( )
}



