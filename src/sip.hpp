#ifndef __SIP_HPP
#define __SIP_HPP
#pragma interface
#include <algorithm>
#include "sdp.hpp"

class PChannel;
//class PSocket;

namespace SIP {

ss :: string trim ( const ss :: string & s );
int atoi ( const ss :: string & s );
int compareNoCase ( const ss :: string & s1, const ss :: string & s2 );

/// Meyer's case-insensitive comparision functor. Can be used for case-insensitive containers
/// e.g: typedef std::map<std::string, int, Utils::CiStringCompare> CiMap;
struct CiStringCompare : public std :: binary_function<ss ::string, ss ::string, bool> {
bool operator()(const ss :: string& lhs, const ss :: string& rhs) const;
};

class Content : public Allocatable < __SS_ALLOCATOR > {
public:

	explicit Content ( int len );

	bool decode ( const ss :: string & str );

	const ss :: string & getContentData ( ) const;
	void setContentData ( const ss :: string & d );

	int getContentLength ( ) const;
	void setContentLength( int );

	const ss :: string & getContentType ( ) const;
	void getContentType ( const ss :: string & d );
protected:
	ss :: string m_ContentData;
	ss :: string m_ContentType;
	int m_ContentLength;
};


class TransportAddress {
public:
	TransportAddress ( const ss :: string & s = "" );
	TransportAddress ( const ss :: string & ip, unsigned short p, const ss :: string & pr = "tcp" );
	ss :: string str ( ) const;
	void printOn ( std :: ostream & os ) const;
	const ss :: string & getAddr ( ) const;
	void setAddr ( const ss :: string & a );
	unsigned short getPort ( ) const;
	void setPort ( unsigned short p );
	void getIpAndPort ( PIPSocket :: Address & ip, unsigned short & port ) const;
private:
	ss :: string proto;
	ss :: string addr;
	unsigned short port;
};

class MediaFormat {
public:
	// the following values are mandated by RFC 2833
	enum NTEEvent {
		Digit0 = 0,
		Digit1 = 1,
		Digit2 = 2,
		Digit3 = 3,
		Digit4 = 4,
		Digit5 = 5,
		Digit6 = 6,
		Digit7 = 7,
		Digit8 = 8,
		Digit9 = 9,
		Star   = 10,
		Hash   = 11,
		A      = 12,
		B      = 13,
		C      = 14,
		D      = 15,
		Flash  = 16
	};

	enum PayloadTypes {
		PCMU,         // G.711 u-Law
		FS1016,       // Federal Standard 1016 CELP
		G721,         // ADPCM - Subsumed by G.726
		G726 = G721,
		GSM,          // GSM 06.10
		G7231,        // G.723.1 at 6.3kbps or 5.3 kbps
		DVI4_8k,      // DVI4 at 8kHz sample rate
		DVI4_16k,     // DVI4 at 16kHz sample rate
		LPC,          // LPC-10 Linear Predictive CELP
		PCMA,         // G.711 A-Law
		G722,         // G.722
		L16_Stereo,   // 16 bit linear PCM
		L16_Mono,     // 16 bit linear PCM
		G723,         // G.723
		CN,           // Confort Noise
		MPA,          // MPEG1 or MPEG2 audio
		G728,         // G.728 16kbps CELP
		DVI4_11k,     // DVI4 at 11kHz sample rate
		DVI4_22k,     // DVI4 at 22kHz sample rate
		G729,         // G.729 8kbps
		Cisco_CN,     // Cisco systems comfort noise (unofficial)

		CelB = 25,    // Sun Systems Cell-B video
		JPEG,         // Motion JPEG
		H261 = 31,    // H.261
		MPV,          // MPEG1 or MPEG2 video
		MP2T,         // MPEG2 transport system
		H263,         // H.263
		T38,	      // my

		LastKnownPayloadType,
		DynamicBase = 96,
		MaxPayloadType = 127,
		ptTelephoneEvent,
		IllegalPayloadType
	};

	MediaFormat ( unsigned payloadType, const ss :: string & name = "",
		unsigned rate = 8000, const ss :: string & param = "", const ss :: string & fmtp = "" );

	void printOn ( std :: ostream & os ) const;

	unsigned getPayloadType ( ) const;

	const ss :: string & getEncodingName ( ) const;
	void setEncodingName ( const ss :: string & v );

	MediaFormat & setFMTP ( const ss :: string & _fmtp );
	void addFMTP ( const ss :: string & key, const ss :: string & val );
	const ss :: string & getFMTP ( ) const;
	ss :: string getPayloadTypePrintable ( ) const;

	void setClockRate ( unsigned v );
	void setParameters ( const ss :: string & v );
	MediaFormat & setPtime ( unsigned pt );
	unsigned getPtime ( ) const;

protected:
	void decodeFMTP ( StringStringMap & m ) const;
	void encodeFMTP ( const StringStringMap & m );
	ss :: string getFMTP ( const ss :: string & key ) const;

	unsigned  payloadType;

	unsigned clockRate;
	ss :: string encodingName;
	ss :: string parameters;
	ss :: string fmtp;
	unsigned ptime;
};

inline std :: ostream & operator<< ( std :: ostream & os, const MediaFormat & m ) {
	m.printOn ( os );
	return os;
}

struct payload { };
typedef boost :: multi_index :: multi_index_container < MediaFormat,
	boost :: multi_index :: indexed_by < boost :: multi_index :: sequenced < >,
	boost :: multi_index :: ordered_unique < boost :: multi_index :: tag < payload >,
	boost :: multi_index :: const_mem_fun < MediaFormat, unsigned, & MediaFormat :: getPayloadType > > >,
	__SS_ALLOCATOR < MediaFormat > > MediaFormatVector;

class MediaDescription {
public:
	enum MediaType {
		mtAudio,
		mtVideo,
		mtImage,
		mtUnknown
	};
	enum TransportType {
		ttRtpAvp,
		ttUdptl,
		ttUnknown
	};
	MediaDescription ( const TransportAddress & address, MediaType mediaType = mtUnknown, TransportType = ttUnknown );
	void printOn ( std :: ostream & os ) const;
	bool decode ( const ss :: string & str );

	MediaType getMediaType ( ) const;
	TransportType getTransportType ( ) const;

	const MediaFormatVector & getMediaFormats ( ) const;
    void setMediaFormats ( const MediaFormatVector & f );

	void addMediaFormat ( const MediaFormat & sdpMediaFormat );

	void setAttribute ( const ss :: string & attr );

	const TransportAddress & getTransportAddress ( ) const;
	void setTransportAddress ( const TransportAddress & t );

	const ss :: string & getTransport ( ) const;
	void setTransport ( const ss :: string & v );

	unsigned getPtime ( ) const;
	void setMaxptime ( unsigned mpt );
	unsigned getMaxptime ( ) const;

    const ss :: string getAuthenticate() const;
    void setAuthenticate(const ss :: string& authenticate);

	unsigned getTelephoneEventsPayloadType() const;
	void setTelephoneEventsPayloadType( unsigned payload );

protected:
    void changeIPInAlt(const SIP::TransportAddress& newIP, ss :: string* alt);

protected:
	ss :: string media;
	ss :: string transport;
	ss :: string pAssertedId_;
	ss :: string authenticate_;
    ss :: string alt_;
	StringStringMap unsupportedAttributes;
	TransportAddress transportAddress;
	MediaFormatVector formats;
	unsigned maxptime;
	unsigned uTelephoneEventsPayloadType_;
	unsigned short portCount;
	MediaType mediaType;
	TransportType transportType;
};

inline std :: ostream & operator<< ( std :: ostream & os, const MediaDescription & m ) {
	m.printOn ( os );
	return os;
}

typedef std :: vector < MediaDescription, __SS_ALLOCATOR < MediaDescription > > MediaDescriptionVector;

class SessionDescription : public Allocatable < __SS_ALLOCATOR > {
public:
	SessionDescription ( );
	SessionDescription ( std :: istream & is );

	void printOn ( std :: ostream & os ) const;
	bool readFrom ( std :: istream & is );
	ss :: string str ( ) const;
	bool decode ( const ss :: string & str );

	void setSessionName ( const ss :: string & v );
	const ss :: string & getSessionName ( ) const;

	void setUserName ( const ss :: string & v );
	const ss :: string & getUserName ( ) const;

	const MediaDescriptionVector & getMediaDescriptions ( ) const;

	const MediaDescription * getMediaDescription ( MediaDescription :: MediaType rtpMediaType ) const;
    void setMediaTransportAddress ();

    MediaDescription * getMediaDescription ( MediaDescription :: MediaType rtpMediaType );
	void addMediaDescription ( const MediaDescription & md );

	const TransportAddress & getDefaultConnectAddress ( ) const;
	void setDefaultConnectAddress ( const TransportAddress & address );

	void setOwnerAddress ( const TransportAddress & o );
	TransportAddress getOwnerAddress ( ) const;

	unsigned getTelephoneEventsPayloadType() const;

protected:
	void parseOwner ( const ss :: string & str );

//    bool sendReqv_;

	MediaDescriptionVector mediaDescriptions;

	ss :: string sessionName;
	ss :: string ownerUsername;
	unsigned protocolVersion;
	unsigned ownerSessionId;
	unsigned ownerVersion;
	unsigned uTelephoneEventsPayloadType_;
	TransportAddress ownerAddress;
	TransportAddress defaultConnectAddress;
};

inline std :: ostream & operator<< ( std :: ostream & os, const SessionDescription & s ) {
	s.printOn ( os );
	return os;
}

inline std :: istream & operator>> ( std :: istream & is, SessionDescription & s ) {
	s.readFrom ( is );
	return is;
}

class URL {
public:
	URL ( );
	URL ( const ss :: string & s );
	URL & operator= ( const URL & u );
	void parse ( const ss :: string & s );
	ss :: string str ( ) const;
	void printOn ( std :: ostream & os ) const;
	ss :: string hostPort ( ) const;
	ss :: string shortForm ( ) const;
	ss :: string bracketShortForm ( ) const;
	enum TranslationType {
		ttLogin,
		ttPath,
		ttQuery,
		ttURL
	};
	static ss :: string untranslateString ( const ss :: string & str, TranslationType type );
	static ss :: string translateString ( const ss :: string & str, TranslationType type );
	const ss :: string & getHostName ( ) const;
	const ss :: string & getUserName ( ) const;
	unsigned short getPort ( ) const;
	void setHostName ( const ss :: string & s );
	void setUserName ( const ss :: string & s );
	void setPort ( unsigned short p );
	void setDisplayName ( const ss :: string & s );
	void setSipParam ( const ss :: string & name, const ss :: string & val );
	const ss :: string getSipParam ( const ss :: string & name ) const;
private:
	ss :: string scheme;
	ss :: string username;
	ss :: string password;
	ss :: string hostname;
	ss :: string pathStr;
	StringVector path;
	StringStringMap paramVars;
	ss :: string fragment;
	StringStringMap queryVars;
	StringStringMap sipParamVars;
	ss :: string displayName;
	unsigned short port;
	bool relativePath;
};

inline std :: ostream & operator<< ( std :: ostream & os, const URL & u ) {
	u.printOn ( os );
	return os;
}

class MIMEInfo {
public:

	enum FieldForm
	{
		ffAbsentIntoMime,
		ffCompactForm,
		ffLongForm,
		ffUnknownForm
	};

	typedef std :: map < ss :: string, ss :: string, CiStringCompare,
		__SS_ALLOCATOR < std :: pair < const ss :: string, ss :: string > > > MapCompactToLongForm;

	explicit MIMEInfo ( bool isRecordRouteRequired, bool compactForm = false );
	MIMEInfo ( bool isRecordRouteRequired, const MIMEInfo & m );
	bool readFrom ( std :: istream & is );
	bool write ( std :: ostream & os ) const;

	void setForm ( bool v );
//	bool getForm() const;

	FieldForm getFieldForm(const ss :: string & fieldName) const;

	ss :: string getContentType ( ) const;
	void setContentType ( const ss :: string & v );

	ss :: string getContentEncoding ( ) const;
	void setContentEncoding ( const ss :: string & v );

	ss :: string getFrom ( ) const;
	void setFrom ( const ss :: string & v );

	ss :: string getCallID ( ) const;
	void setCallID ( const ss :: string & v );

	ss :: string getContact ( ) const;
	void setContact ( const ss :: string & v );
	void setContact ( const URL & url);

	ss :: string getSubject ( ) const;
	void setSubject ( const ss :: string & v );

	ss :: string getTo ( ) const;
	void setTo ( const ss :: string & v );

	ss :: string getVia ( ) const;
	void setVia ( const ss :: string & v );

	unsigned getContentLength ( ) const;
	void setContentLength ( unsigned v );

	ss :: string getCSeq ( ) const;
	void setCSeq ( const ss :: string & v );

	StringVector getRoute ( ) const;
	void setRoute ( const StringVector & v );

	StringVector getRecordRoute ( ) const;
	void setRecordRoute ( const StringVector & v );
	void addRecordRoute(const ss :: string& url, bool ins = true);
	void clearRecordRoute();

	void setRoute(const ss :: string& url, bool ins = true);

	unsigned getCSeqIndex ( ) const;

	int getMaxForwards ( ) const;
	void setMaxForwards ( int v );
	bool updateMaxForwards ( const MIMEInfo & m );

	ss :: string getAccept ( ) const;
	void setAccept ( const ss :: string & v );

	ss :: string getUserAgent ( ) const;
	void setUserAgent ( const ss :: string & v );

	ss :: string getAcceptEncoding ( ) const;
	void setAcceptEncoding ( const ss :: string & v );

	ss :: string getAcceptLanguage ( ) const;
	void setAcceptLanguage ( const ss :: string & v );

	ss :: string getSupported ( ) const;
	void setSupported ( const ss :: string & v );

	ss :: string getAllow ( ) const;
	void setAllow ( const ss :: string & v );

 	ss :: string getAuthorization ( ) const;
	void setAuthorization ( const ss :: string & v );

	ss :: string getWWWAuthenticate ( ) const;
	void setWWWAuthenticate ( const ss :: string & v );

	ss :: string getProxyAuthorization ( ) const;
	void setProxyAuthorization ( const ss :: string & v );

	ss :: string getProxyAuthenticate ( ) const;
	void setProxyAuthenticate ( const ss :: string & v );

	ss :: string getExpires ( ) const;
	void setExpires ( const ss :: string & v );

	void setPAssertID( const ss :: string & newAssertId );
	ss :: string getPAssertID() const;
	void removePAssertID( );

	void setPrintPAssertId(bool isPrint);

	bool getRecordRouteRequired() const;
	void setRecordRouteRequired(bool isRecordRouteRequired);

	ss :: string getContactString() const;
	void addPrivacyHeader();

	ss :: string getRealFrom() const;

    ss :: string getRemotePartyID() const;
    void setRemotePartyID( const ss :: string& remotePartyID );
    ss :: string getRemotePartyId() const;
    void setRemotePartyId( const ss :: string& remotePartyID );
        void setPrivacy( const ss :: string& url );
	ss :: string getPrivacy( ) const;
		
		
protected:
	StringVector getRouteList ( const ss :: string & name ) const;
	void printRecordRoute ( std :: ostream & os ) const;
	void setRouteList ( const ss :: string & name, const StringVector & v );
	ss :: string getFullOrCompact ( const ss :: string & fullForm, char compactForm ) const;
	ss :: string get ( const ss :: string & name ) const;
	void set ( const ss :: string & name, const ss :: string & val );
	bool addMIME ( const ss :: string & line );

	void fillFormMap();
	ss :: string getFormString(const ss :: string& name) const;

	struct attribute { };
	typedef std :: pair < ss :: string, ss :: string > value_t;
	typedef boost :: multi_index :: multi_index_container <
		value_t,
		boost :: multi_index :: indexed_by <
			boost :: multi_index :: sequenced < >,
			boost :: multi_index :: ordered_unique <
				boost :: multi_index :: tag < attribute >,
				boost :: multi_index :: member < value_t, value_t :: first_type, & value_t :: first > ,
				CiStringCompare>
			>,
		__SS_ALLOCATOR < value_t >
	> values_t;

	values_t values;
	ss :: string strPAsserId_;
		/// Encode using compact form

	MapCompactToLongForm compactFormMap_;
	MapCompactToLongForm longFormMap_;
    StringVector vectorRoute_;
    ss :: string strRoute_;
    ss :: string strContact_;
    ss :: string strRealFrom_;

    ss :: string m_remotePartyID;

	bool compactForm;
    bool isRecordRouteRequired_;
    bool isPrivecyHeader_;
};

inline std :: istream & operator>> ( std :: istream & is, MIMEInfo & m ) {
	m.readFrom ( is );
	return is;
}

inline std :: ostream & operator<< ( std :: ostream & os, MIMEInfo & m ) {
	m.write ( os );
	return os;
}

class PDU {
public:
	enum Methods {
		mInvite,
		mAck,
		mOptions,
		mBye,
		mCancel,
		mRegister,
        mRefer,
        mMessage,
        mNotify,
        mSubscribe,
        mInfo,
//		mUnAuthorized,
		mNum
	};
	enum StatusCodes {
		scIllegal,
		scInformationTrying		= 100,
		scInformationDialogEstablishement = 101,

		scInformationRinging		= 180,
		scInformationCallForwarded	= 181,
		scInformationQueued		= 182,
		scInformationSessionProgress	= 183,

		scSuccessfulOK			= 200,

		scAccepted	= 202,
		scRedirectionMovedTemporarily	= 302,

		scFailureBadRequest		= 400,
		scFailureUnAuthorised		= 401,
		scFailurePaymentRequired	= 402,
		scFailureForbidden		= 403,
		scFailureNotFound		= 404,
		scFailureMethodNotAllowed	= 405,
		scFailureNotAcceptable		= 406,
		scFailureProxyAuthRequired	= 407,
		scFaiureRequestTimeout		= 408,
		scFaiureConflict		= 409,
		scFaiureGone			= 410,
		scFaiureLengthRequired		= 411,
		scFaiureRequestEntityTooLarge	= 413,
		scFaiureRequestURITooLong	= 414,
		scFaiureUnsupportedMediaType	= 415,
		scFaiureBadExtension		= 420,
		scFaiureTemporarilyUnavailable	= 480,
		scFaiureTransactionDoesNotExist	= 481,
		scFaiureLoopDetected		= 482,
		scFaiureTooManyHops		= 483,
		scFaiureAddressIncomplete	= 484,
		scFaiureAmbiguous		= 485,
		scFaiureBusyHere		= 486,
		scFaiureRequestTerminated	= 487,

		scFaiureBadGateway		= 502,
		scFaiureServiceUnavailable = 503,

		scFaiureDecline			= 603,

		scMax				= 699
	};

	enum {
		maxSize = 65535
	};

	PDU ( );
	PDU ( const PDU & );
	PDU ( Methods m, const URL & u );
	PDU & operator = ( const PDU & );
	~PDU ( );
	bool read ( IxcUDPSocket & sock );
	bool readFrom ( std :: istream & is );

	bool write ( PChannel & transport );
	void printOn ( std :: ostream & os );

	ss :: string getTransactionID ( ) const;
	int getStatusCode ( ) const;
	void setStatusCode ( int sc );
	Methods getMethod ( ) const;
	void setMethod ( Methods m );
//	StatusCodes getStatusCode ( ) const;
//	void setStatusCode ( StatusCodes sc );

	const URL & getURI ( ) const;
	void setURI ( const URL & u );
	unsigned getVersionMajor ( ) const;
	unsigned getVersionMinor ( ) const;
	const ss :: string & getEntityBody ( ) const;
	void setEntityBody ( const ss :: string & e );
	const ss :: string & getInfo ( ) const;
	void setInfo ( const ss :: string & i );
	const MIMEInfo & getMIME ( ) const;
	MIMEInfo & getMIME ( );
	void setMIME ( const MIMEInfo & m );
	bool hasSDP ( ) const;
	SessionDescription & getSDP ( ) const;
	void setSDP ( SessionDescription * s );
	void setSDP ( const SessionDescription & s );

	void setContact ( const ss :: string & strContact );

	const Content * getContent ( ) const;

protected:
	Methods method;
//	StatusCodes statusCode;
	int statusCode;
	URL uri_;
	unsigned versionMajor;
	unsigned versionMinor;
	ss :: string info;
	MIMEInfo mime;
	ss :: string entityBody;
	SessionDescription * sdp;

	Content * m_Content;
	ss :: string strContact_;
};

inline std :: ostream & operator<< ( std :: ostream & os, PDU & p ) {
	p.printOn ( os );
	return os;
}

inline std :: istream & operator>> ( std :: istream & is, PDU & p ) {
	p.readFrom ( is );
	return is;
}

}

#endif  // __SIP_HPP
