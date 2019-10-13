#ifndef SIP2_HPP_
#define SIP2_HPP_
#pragma interface

namespace SDP {
class SessionDescription;
}

namespace DTMF {
struct Relay;
}

namespace boost {
namespace posix_time {
class ptime;
}
}

namespace SIP2 {

class URI {
	class Impl;
	Impl * impl;
public:
	URI ( );
	explicit URI ( const ss :: string & s );
	~URI ( );
	URI ( const URI & u );
	URI & operator= ( const URI & u ) {
		URI ( u ).swap ( * this );
		return * this;
	}
	void swap ( URI & u ) {
		std :: swap ( impl, u.impl );
	}
	void printOn ( std :: ostream & os ) const;
	void setUser ( const ss :: string & u );
	void setUser ( const ss :: string :: const_iterator & b, const ss :: string :: const_iterator & e );
	const ss :: string & getUser ( ) const;
	void setPassword ( const ss :: string & p );
	void setPassword ( const ss :: string :: const_iterator & b, const ss :: string :: const_iterator & e );
	void setHost ( const ss :: string & h );
	void setHost ( const ss :: string :: const_iterator & b, const ss :: string :: const_iterator & e );
	const ss :: string & getHost ( ) const;
	void setPort ( unsigned p );
	unsigned getPort ( ) const;
	void setParameter ( const ss :: string & pname, const ss :: string & pval );
	bool hasParameter ( const ss :: string & n ) const;
	void delParameter ( const ss :: string & pname );
	void addHeader ( const ss :: string hname, const ss :: string & hval );
	void clearHeaders ( );
};

inline std :: ostream & operator<< ( std :: ostream & os, const URI & u ) {
	u.printOn ( os );
	return os;
}


class RouteHeader {
	class Impl;
	Impl * impl;
	public:
	RouteHeader ( );
	~RouteHeader ( );
	RouteHeader ( const RouteHeader & n );
	RouteHeader & operator= ( const RouteHeader & n ) {
		RouteHeader ( n ).swap ( * this );
		return * this;
	}
	void swap ( RouteHeader & n ) {
		std :: swap ( impl, n.impl );
	}
	void printOn ( std :: ostream & os ) const;
	void setUri ( const URI & u );
	const URI & getUri ( ) const;
	void setDisplayName ( const ss :: string & n );
	void setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue );
};

typedef std :: vector < RouteHeader, __SS_ALLOCATOR < RouteHeader > > RouteHeaderVector;

inline std :: ostream & operator<< ( std :: ostream & os, const RouteHeader & u ) {
	u.printOn ( os );
	return os;
}

class FromToHeader {
	class Impl;
	Impl * impl;
	public:
	FromToHeader ( );
	explicit FromToHeader ( const ss :: string & s );
	~FromToHeader ( );
	FromToHeader ( const FromToHeader & n );
	FromToHeader & operator= ( const FromToHeader & n ) {
		FromToHeader ( n ).swap ( * this );
		return * this;
	}
	void swap ( FromToHeader & n ) {
		std :: swap ( impl, n.impl );
	}
	void printOn ( std :: ostream & os ) const;
	void setUri ( const URI & u );
	const URI & getUri ( ) const;
	void setDisplayName ( const ss :: string & n );
	void setTag ( const ss :: string & t );
	const ss :: string & getTag ( ) const;
	void setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue );
};

inline std :: ostream & operator<< ( std :: ostream & os, const FromToHeader & u ) {
	u.printOn ( os );
	return os;
}

class ContactHeader {
	class Impl;
	Impl * impl;
	public:
	ContactHeader ( );
	explicit ContactHeader ( const ss :: string & s );
	~ContactHeader ( );
	ContactHeader ( const ContactHeader & n );
	ContactHeader & operator= ( const ContactHeader & n ) {
		ContactHeader ( n ).swap ( * this );
		return * this;
	}
	void swap ( ContactHeader & n ) {
		std :: swap ( impl, n.impl );
	}
	void printOn ( std :: ostream & os ) const;
	void setUri ( const URI & u );
	const URI & getUri ( ) const;
	void setDisplayName ( const ss :: string & n );
	void setQ ( const ss :: string & t );
	void setExpires ( unsigned t );
	bool hasExpires ( ) const;
	unsigned getExpires ( ) const;
	void setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue );
};

inline std :: ostream & operator<< ( std :: ostream & os, const ContactHeader & u ) {
	u.printOn ( os );
	return os;
}

class Via {
	class Impl;
	Impl * impl;
public:
	Via ( const Via & v );
	~Via ( );
	Via ( );
	Via & operator= ( const Via & v ) {
		Via ( v ).swap ( * this );
		return * this;
	}
	void swap ( Via & v ) {
		std :: swap ( impl, v.impl );
	}
	void printOn ( std :: ostream & os ) const;
	void setTransport ( const ss :: string & t );
	void setHost ( const ss :: string & h );
	const ss :: string & getHost ( ) const;
	void setPort ( int p );
	int getPort ( ) const;
	void setRport ( int p = 0 );
	const boost :: optional < int > & getRport ( ) const;
	void setBranch ( const ss :: string & b );
	const ss :: string & getBranch ( ) const;
	void setReceived ( const ss :: string & r );
	const ss :: string & getReceived ( ) const;
	void setUnknownParameter ( const ss :: string & pname, const ss :: string & pvalue );
};

inline const ss :: string & getFromAddr ( const Via & v ) {
	const ss :: string & s = v.getReceived ( );
	if ( ! s.empty ( ) )
		return s;
	return v.getHost ( );
}

inline int getFromPort ( const Via & v ) {
	if ( const boost :: optional < int > & p = v.getRport ( ) )
		return * p;
	if ( int p = v.getPort ( ) )
		return p;
	return 5060;
}

typedef std :: vector < Via, __SS_ALLOCATOR < Via > > ViaVector;

inline std :: ostream & operator<< ( std :: ostream & os, const Via & v ) {
	v.printOn ( os );
	return os;
}

class DigestChallenge {
	class Impl;
	Impl * impl;
public:
	DigestChallenge ( const ss :: string & realm, const ss :: string & nonce, const StringSet & qop );
	DigestChallenge ( const DigestChallenge & v );
	explicit DigestChallenge ( const ss :: string & v );
	~DigestChallenge ( );
	DigestChallenge & operator= ( const DigestChallenge & v ) {
		DigestChallenge ( v ).swap ( * this );
		return * this;
	}
	void swap ( DigestChallenge & v ) {
		std :: swap ( impl, v.impl );
	}
	void printOn ( std :: ostream & os ) const;
	const ss :: string & getAlgorithm ( ) const;
	void setRealm ( const ss :: string & r );
	const ss :: string & getRealm ( ) const;
	void setNonce ( const ss :: string & n );
	const ss :: string & getNonce ( ) const;
	const ss :: string & getOpaque ( ) const;
	bool hasQop ( const ss :: string & q ) const;
	bool emptyQop ( ) const;
	void setStale ( bool s = true );
};

typedef std :: vector < DigestChallenge, __SS_ALLOCATOR < DigestChallenge > > DigestChallengeVector;

inline std :: ostream & operator<< ( std :: ostream & os, const DigestChallenge & v ) {
	v.printOn ( os );
	return os;
}

class Request;

class DigestResponse {
	class Impl;
	Impl * impl;
public:
	DigestResponse ( const DigestResponse & v );
	explicit DigestResponse ( const ss :: string & v );
	DigestResponse ( const DigestChallenge & challenge, const Request & r, unsigned nc, const ss :: string & username, const ss :: string & pass );
	~DigestResponse ( );
	DigestResponse & operator= ( const DigestResponse & v ) {
		DigestResponse ( v ).swap ( * this );
		return * this;
	}
	void swap ( DigestResponse & v ) {
		std :: swap ( impl, v.impl );
	}
	void printOn ( std :: ostream & os ) const;
	ss :: string calcDigest ( const ss :: string & pass, const Request & r ) const;
	ss :: string hash ( const ss :: string & s ) const;
	ss :: string a1 ( const ss :: string & pass ) const;
	const ss :: string & getUsername ( ) const;
	const ss :: string & getRealm ( ) const;
	const ss :: string & getNonce ( ) const;
	const ss :: string & getCnonce ( ) const;
	const ss :: string & getQop ( ) const;
	const URI & getUri ( ) const;
	unsigned getNc ( ) const;
	const ss :: string & getResponse ( ) const;
};

typedef std :: vector < DigestResponse, __SS_ALLOCATOR < DigestResponse > > DigestResponseVector;

inline std :: ostream & operator<< ( std :: ostream & os, const DigestResponse & v ) {
	v.printOn ( os );
	return os;
}

class Response;

class AuthenticationInfo {
	class Impl;
	Impl * impl;
public:
	AuthenticationInfo ( const AuthenticationInfo & v );
	explicit AuthenticationInfo ( const ss :: string & v );
	AuthenticationInfo ( const DigestResponse & v, const Response & r, const ss :: string & pass );
	~AuthenticationInfo ( );
	AuthenticationInfo & operator= ( const AuthenticationInfo & v ) {
		AuthenticationInfo ( v ).swap ( * this );
		return * this;
	}
	void swap ( AuthenticationInfo & v ) {
		std :: swap ( impl, v.impl );
	}
	void printOn ( std :: ostream & os ) const;
	ss :: string calcDigest ( const ss :: string & pass, const Response & r, const DigestResponse & d ) const;
	void setNextnonce ( const ss :: string & s );
	const ss :: string & getNextnonce ( ) const;
	unsigned getNc ( ) const;
	void setRspauth ( const ss :: string & s );
	const ss :: string & getRspauth ( ) const;
	const ss :: string & getCnonce ( ) const;
};

inline std :: ostream & operator<< ( std :: ostream & os, const AuthenticationInfo & v ) {
	v.printOn ( os );
	return os;
}

class RetryAfter {
	class Impl;
	Impl * impl;
public:
	RetryAfter ( const RetryAfter & v );
	explicit RetryAfter ( const ss :: string & v );
	explicit RetryAfter ( unsigned secs );
	~RetryAfter ( );
	RetryAfter & operator= ( const RetryAfter & v ) {
		RetryAfter ( v ).swap ( * this );
		return * this;
	}
	void swap ( RetryAfter & v ) {
		std :: swap ( impl, v.impl );
	}
	void printOn ( std :: ostream & os ) const;
	unsigned getSeconds ( ) const;
	void setDuration ( unsigned dur );
	unsigned getDuration ( ) const;
};

inline std :: ostream & operator<< ( std :: ostream & os, const RetryAfter & v ) {
	v.printOn ( os );
	return os;
}

class Warning {
	class Impl;
	Impl * impl;
public:
	Warning ( const Warning & v );
	Warning ( unsigned c, const ss :: string & a, const ss :: string & t );
	~Warning ( );
	Warning & operator= ( const Warning & v ) {
		Warning ( v ).swap ( * this );
		return * this;
	}
	void swap ( Warning & v ) {
		std :: swap ( impl, v.impl );
	}
	void printOn ( std :: ostream & os ) const;
};

typedef std :: vector < Warning, __SS_ALLOCATOR < Warning > > WarningVector;

inline std :: ostream & operator<< ( std :: ostream & os, const Warning & v ) {
	v.printOn ( os );
	return os;
}

class SessionExpires {
	class Impl;
	Impl * impl;
public:
	SessionExpires ( const SessionExpires & v );
	explicit SessionExpires ( unsigned s, const ss :: string & r = ss :: string ( ) );
	explicit SessionExpires ( const ss :: string & s );
	~SessionExpires ( );
	SessionExpires & operator= ( const SessionExpires & s ) {
		SessionExpires ( s ).swap ( * this );
		return * this;
	}
	void swap ( SessionExpires & v ) {
		std :: swap ( impl, v.impl );
	}
	void printOn ( std :: ostream & os ) const;
};

inline std :: ostream & operator<< ( std :: ostream & os, const SessionExpires & s ) {
	s.printOn ( os );
	return os;
}

class ContentType {
	class Impl;
	Impl * impl;
public:
	ContentType ( const ContentType & v );
	explicit ContentType ( const ss :: string & s );
	~ContentType ( );
	ContentType & operator= ( const ContentType & s ) {
		ContentType ( s ).swap ( * this );
		return * this;
	}
	void swap ( ContentType & v ) {
		std :: swap ( impl, v.impl );
	}
	void printOn ( std :: ostream & os ) const;
	const ss :: string & getType ( ) const;
	const ss :: string & getSubType ( ) const;
};

inline std :: ostream & operator<< ( std :: ostream & os, const ContentType & s ) {
	s.printOn ( os );
	return os;
}

class AcceptRange : public Allocatable < __SS_ALLOCATOR > {
	ss :: string type, subType;
public:
	AcceptRange ( const ss :: string & type, const ss :: string & subType ) : type ( type ), subType ( subType ) { }
	template < typename Iterator > AcceptRange ( const Iterator & b1, const Iterator & e1, const Iterator & b2,
		const Iterator & e2 ) : type ( b1, e1 ), subType ( b2, e2 ) { }
	void printOn ( std :: ostream & os ) const {
		os << type << '/' << subType;
	}
	void swap ( AcceptRange & a ) {
		type.swap ( a.type );
		subType.swap ( a.subType );
	}
	const ss :: string & getType ( ) const {
		return type;
	}
	const ss :: string & getSubType ( ) const {
		return subType;
	}
};

inline std :: ostream & operator<< ( std :: ostream & os, const AcceptRange & s ) {
	s.printOn ( os );
	return os;
}

inline bool operator< ( const AcceptRange & a, const AcceptRange & b ) {
	if ( int r = a.getType ( ).compare ( b.getType ( ) ) )
		return r < 0;
	return a.getSubType ( ) < b.getSubType ( );
}

typedef std :: set < AcceptRange, std :: less < AcceptRange >, __SS_ALLOCATOR < AcceptRange > > AcceptRangeSet;

struct CSeq;

class Dialog;

class RequestMIMEInfo;

class Request {
	class Impl;
	Impl * impl;
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
		mUpdate,
		mPrack,
		mPublish,
		mNumMethods
	};
	Request ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie );
	Request ( Dialog & d, Methods m, int snum = 0, ss :: string branch = "" ); //request inside dialog
	Request ( const ss :: string & host, const ss :: string & user, int port, const ss :: string & localHost,
		const ss :: string & localUser, const ss :: string & callId, unsigned maxForwards, unsigned cseq,
		SDP :: SessionDescription * sdp ); //invite
	Request ( const ss :: string & host, const ss :: string & user, int port, const ss :: string & callId, unsigned seq );
	//register
	Request ( const Request & invite, const Response & r ); // ack for non-2xx
	Request ( const Request & m );
	~Request ( );
	Request & operator= ( const Request & m ) {
		Request ( m ).swap ( * this );
		return * this;
	}
	void swap ( Request & m ) {
		std :: swap ( impl, m.impl );
	}
	void printOn ( std :: ostream & os ) const;
	RequestMIMEInfo & getMime ( );
	const RequestMIMEInfo & getMime ( ) const;
	Methods getMethod ( ) const;
	const URI & getRequestUri ( ) const;
	void setSdp ( SDP :: SessionDescription * s );
	const SDP :: SessionDescription * getSdp ( ) const;
	void setDtmfRelay ( const DTMF :: Relay & r, bool supportsPackage );
	const DTMF :: Relay * getDtmfRelay ( ) const;
	bool emptyBody ( ) const;
	const ss :: string & getOrigBodyMd5 ( ) const;
	void printBody ( std :: ostream & os ) const;
};

inline std :: ostream & operator<< ( std :: ostream & os, const Request & r ) {
	r.printOn ( os );
	return os;
}

class ResponseMIMEInfo;

class Response {
	class Impl;
	Impl * impl;
public:
	enum StatusCodes {
		scInformationTrying		= 100,
		scInformationDialogEstablishement = 101,

		scInformationRinging		= 180,
		scInformationCallForwarded	= 181,
		scInformationQueued		= 182,
		scInformationSessionProgress	= 183,

		scSuccessfulOK			= 200,
		scAccepted			= 202, //netu


		scRedirectionMultipleChoices	= 300,
		scRedirectionMovedPermanently	= 301,
		scRedirectionMovedTemporarily	= 302,
		scRedirectionUseProxy		= 305,
		scRedirectionAlternativeService	= 380,

		scFailureBadRequest		= 400,
		scFailureUnAuthorized		= 401,
		scFailurePaymentRequired	= 402,
		scFailureForbidden		= 403,
		scFailureNotFound		= 404,
		scFailureMethodNotAllowed	= 405,
		scFailureNotAcceptable		= 406,
		scFailureProxyAuthRequired	= 407,
		scFailureRequestTimeout		= 408,
		scFailureConflict		= 409, //netu
		scFailureGone			= 410,
		scFailureLengthRequired		= 411, //netu
		scFailureRequestEntityTooLarge	= 413,
		scFailureRequestURITooLong	= 414,
		scFailureUnsupportedMediaType	= 415,
		scFailureUnsupportedURIScheme	= 416,
		scFailureBadExtension		= 420,
		scFailureExtensionRequired	= 421,
		scFailureSessionIntervalTooSmall	= 422,
		scFailureIntervalTooBrief	= 423,
		scFailureTemporarilyUnavailable	= 480,
		scFailureTransactionDoesNotExist	= 481,
		scFailureLoopDetected		= 482,
		scFailureTooManyHops		= 483,
		scFailureAddressIncomplete	= 484,
		scFailureAmbiguous		= 485,
		scFailureBusyHere		= 486,
		scFailureRequestTerminated	= 487,
		scFailureNotAcceptableHere	= 488,
		scFailureBadEvent		= 489,
		scFailureRequestPending		= 491,
		scFailureUndecipherable		= 493,

		scFailureServerInternalError	= 500,
		scFailureNotImplemented		= 501,
		scFailureBadGateway		= 502,
		scFailureServiceUnavailable	= 503,
		scFailureServerTimeout		= 504,
		scFailureVersionNotSupported	= 505,
		scFailureMessageTooLarge	= 513,

		scFaiulreBusyEverywhere		= 600,
		scFaiulreDecline		= 603,
		scFaiulreDoesNotExistAnywhere	= 604,
		scFaiulreNotAcceptable		= 606,

		scMax				= 699
	};
	Response ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie );
	Response ( const Response & r );
	Response ( const Request & r, StatusCodes c, const ss :: string & tag = ss :: string ( ),
		const ss :: string & rf = ss :: string ( ) );
	Response ( const Request & r, StatusCodes c, const Dialog & d, const ss :: string & rf = ss :: string ( ) );
	~Response ( );
	Response & operator= ( const Response & r ) {
		Response ( r ).swap ( * this );
		return * this;
	}
	void swap ( Response & r ) {
		std :: swap ( impl, r.impl );
	}
	void printOn ( std :: ostream & os ) const;
	ResponseMIMEInfo & getMime ( );
	const ResponseMIMEInfo & getMime ( ) const;
	int getStatusCode ( ) const;
	const ss :: string & getReasonPhrase ( ) const;
	const SDP :: SessionDescription * getSdp ( ) const;
	void setSdp ( SDP :: SessionDescription * sdp );
	const ss :: string & getOrigBodyMd5 ( ) const;
	void printBody ( std :: ostream & os ) const;
};

inline std :: ostream & operator<< ( std :: ostream & os, const Response & r ) {
	r.printOn ( os );
	return os;
}

enum InfoPackages {
	ipNil,
	ipDtmf,
	ipNumPackages
};

struct ContentDisposition {
	enum Disposition {
		missing,
		alert,
		attachment,
		icon,
		infoPackage,
		render,
		session,
		signal
	} __attribute__ (( packed )) disposition;
	bool handlingRequired;
	ContentDisposition ( ) : disposition ( missing ) { }
	ContentDisposition ( Disposition d, bool r ) : disposition ( d ), handlingRequired ( r ) { }
	void printOn ( std :: ostream & os ) const;
};

std :: ostream & operator<< ( std :: ostream & os, ContentDisposition d );

struct RAck;

class RequestMIMEInfo {
	class Impl;
	Impl * impl;
public:
	RequestMIMEInfo ( const RequestMIMEInfo & m );
	~RequestMIMEInfo ( );
	RequestMIMEInfo ( );
	RequestMIMEInfo & operator= ( const RequestMIMEInfo & m ) {
		RequestMIMEInfo ( m ).swap ( * this );
		return * this;
	}
	void swap ( RequestMIMEInfo & m ) {
		std :: swap ( impl, m.impl );
	}
	void printOn ( std :: ostream & os ) const;
	void addAccept ( const ss :: string & s );
	void addAccept ( const AcceptRange & a );
	const boost :: optional < AcceptRangeSet > & getAccept ( ) const;
	void setAllow ( const ss :: string & s );
	void setAllow ( unsigned a );
	bool getAllow ( Request :: Methods m ) const;
	void addAuthorization ( const DigestResponse & s );
	const DigestResponseVector & getAuthorization ( ) const;
	void setCallId ( const ss :: string & s );
	const ss :: string & getCallId ( ) const;
	void setContact ( const ss :: string & s );
	void setContact ( const ContactHeader & s );
	const ContactHeader & getContact ( ) const;
	void setContentDisposition ( const ss :: string & s );
	void setContentDisposition ( const ContentDisposition & d );
	ContentDisposition getContentDisposition ( ) const;
	void setCseq ( const ss :: string & s );
	void setCseq ( const CSeq & s );
	const CSeq & getCseq ( ) const;
	void setExpires ( unsigned e );
	bool hasExpires ( ) const;
	unsigned getExpires ( ) const;
	void setFrom ( const ss :: string & s );
	void setFrom ( const FromToHeader & s );
	const FromToHeader & getFrom ( ) const;
	void setInfoPackage ( InfoPackages p );
	void setInfoPackage ( const ss :: string & s );
	InfoPackages getInfoPackage ( ) const;
	void setMaxForwards ( unsigned s );
	unsigned getMaxForwards ( ) const;
	void setMinSE ( unsigned d );
	unsigned getMinSE ( ) const;
	void setPAssertID ( const ss :: string & s );
	const ss :: string & getPAssertID ( ) const;
	void setPrivacy ( const ss :: string & s );
	const ss :: string & getPrivacy ( ) const;
	void addProxyAuthorization ( const DigestResponse & s );
	void setRAck ( const RAck & r );
	void addRecordRoute ( const ss :: string & s );
	void addRecordRoute ( const RouteHeader & s );
	const RouteHeaderVector & getRecordRoute ( ) const;
	void setRecvInfo ( const ss :: string & s );
	void setRecvInfo ( unsigned a );
	bool getRecvInfo ( InfoPackages p ) const;
	bool hasRecvInfo ( ) const;
	void setRequire ( const ss :: string & s );
	void addRequire ( const ss :: string & s );
	const StringVector & getRequire ( ) const;
	void addRoute ( const ss :: string & s );
	void addRoute ( const RouteHeader & s );
	void setRoute ( const RouteHeaderVector & v );
	const RouteHeaderVector & getRoute ( ) const;
	void setRemotePartyID ( const ss :: string & s );
	const ss :: string & getRemotePartyID ( ) const;
	void setSessionExpires ( const SessionExpires & s );
	void setSessionExpires ( unsigned d, const ss::string & r );
	void setSessionExpires ( const ss::string & s );
	bool hasSessionExpires ( ) const;
	const SessionExpires & getSessionExpires ( ) const;
	void setSupported ( const ss :: string & s );
	void addSupported ( const ss :: string & s );
	const StringVector & getSupported ( ) const;
	void setTimestamp ( const ss :: string & s );
	const ss :: string & getTimestamp ( ) const;
	void setTo ( const ss :: string & s );
	void setTo ( const FromToHeader & s );
	const FromToHeader & getTo ( ) const;
	void setUserAgent ( const ss :: string & s );
	const ss :: string & getUserAgent ( ) const;
	void addVia ( const ss :: string & s );
	void addVia ( const Via & s );
	const ViaVector & getVia ( ) const;
	ViaVector & getVia ( );
};

inline std :: ostream & operator<< ( std :: ostream & os, const RequestMIMEInfo & m ) {
	m.printOn ( os );
	return os;
}

class ResponseMIMEInfo {
	class Impl;
	Impl * impl;
public:
	ResponseMIMEInfo ( const ResponseMIMEInfo & m );
	~ResponseMIMEInfo ( );
	ResponseMIMEInfo ( );
	ResponseMIMEInfo & operator= ( const ResponseMIMEInfo & m ) {
		ResponseMIMEInfo ( m ).swap ( * this );
		return * this;
	}
	void swap ( ResponseMIMEInfo & m ) {
		std :: swap ( impl, m.impl );
	}
	void printOn ( std :: ostream & os ) const;
	void addAccept ( const ss :: string & s );
	void addAccept ( const AcceptRange & a );
	const boost :: optional < AcceptRangeSet > & getAccept ( ) const;
	void setAllow ( const ss :: string & s );
	void setAllow ( unsigned a );
	bool getAllow ( Request :: Methods m ) const;
	void setAuthenticationInfo ( const AuthenticationInfo & s );
	void setAuthenticationInfo ( const ss :: string & s );
	bool hasAuthenticationInfo ( ) const;
	const AuthenticationInfo & getAuthenticationInfo ( ) const;
	void setCallId ( const ss :: string & s );
	const ss :: string & getCallId ( ) const;
	void setContact ( const ss :: string & s );
	void setContact ( const ContactHeader & s );
	const ContactHeader & getContact ( ) const;
	void setContentDisposition ( const ss :: string & s );
	void setContentDisposition ( const ContentDisposition & d );
	ContentDisposition getContentDisposition ( ) const;
	void setCseq ( const ss :: string & s );
	void setCseq ( const CSeq & s );
	const CSeq & getCseq ( ) const;
	void setDate ( const boost :: posix_time :: ptime & s );
	void setExpires ( unsigned e );
	bool hasExpires ( ) const;
	unsigned getExpires ( ) const;
	void setFrom ( const ss :: string & s );
	void setFrom ( const FromToHeader & s );
	const FromToHeader & getFrom ( ) const;
	void setMinExpires ( unsigned d );
	unsigned getMinExpires ( ) const;
	void setMinSE ( unsigned d );
	unsigned getMinSE ( ) const;
	void addProxyAuthenticate ( const DigestChallenge & s );
	void addRecordRoute ( const ss :: string & s );
	void addRecordRoute ( const RouteHeader & s );
	const RouteHeaderVector & getRecordRoute ( ) const;
	void setRecvInfo ( const ss :: string & s );
	void setRecvInfo ( unsigned a );
	bool getRecvInfo ( InfoPackages p ) const;
	bool hasRecvInfo ( ) const;
	void setRequire ( const ss :: string & s );
	void setRSeq ( unsigned r );
	void addRequire ( const ss :: string & s );
	const StringVector & getRequire ( ) const;
	void setRetryAfter ( const RetryAfter & s );
	void addRoute ( const ss :: string & s );
	void addRoute ( const RouteHeader & s );
	void setServer ( const ss :: string & s );
	void setSessionExpires ( const SessionExpires & s );
	void setSessionExpires ( unsigned d, const ss::string & r );
	void setSessionExpires ( const ss::string & s );
	bool hasSessionExpires ( ) const;
	const SessionExpires & getSessionExpires ( ) const;
	void setSupported ( const ss :: string & s );
	void addSupported ( const ss :: string & s );
	const StringVector & getSupported ( ) const;
	void setTimestamp ( const ss :: string & s );
	void setTo ( const ss :: string & s );
	void setTo ( const FromToHeader & s );
	const FromToHeader & getTo ( ) const;
	void setUnsupported ( const ss :: string & s );
	void addUnsupported ( const ss :: string & s );
	const StringVector & getUnsupported ( ) const;
	void addVia ( const ss :: string & s );
	void addVia ( const Via & s );
	const ViaVector & getVia ( ) const;
	void addWarning ( const ss :: string & s );
	void addWarning ( const Warning & s );
	void addWwwAuthenticate ( const DigestChallenge & s );
	void setRemotePartyID ( const ss :: string & s );
	const ss :: string & getRemotePartyID ( ) const;
	const DigestChallengeVector & getWwwAuthenticate ( ) const;
};

inline std :: ostream & operator<< ( std :: ostream & os, const ResponseMIMEInfo & m ) {
	m.printOn ( os );
	return os;
}

std :: istream & operator>> ( std :: istream & is, CSeq & s );

struct CSeq {
	Request :: Methods method;
	int seq;
	explicit CSeq ( Request :: Methods m = Request :: mInvite, int s = 1 ) : method ( m ), seq ( s ) { }
	explicit CSeq ( const ss :: string & s ) {
		ss :: istringstream is ( s );
		is >> * this;
	}

};

inline bool operator!= ( const CSeq & s1, const CSeq & s2 ) {
	return s1.method != s2.method || s1.seq != s2.seq;
}

inline bool operator< ( const CSeq & s1, const CSeq & s2 ) {
	if ( s1.method == s2.method )
		return s1.seq < s2.seq;
	return s1.method < s2.method;
}

std :: ostream & operator<< ( std :: ostream & os, const CSeq & s );

struct RAck {
	unsigned responseNum;
	CSeq cseq;
	RAck ( ) : responseNum ( 0 ) { }
	explicit RAck ( const ss :: string & s ) {
		ss :: istringstream is ( s );
		is >> responseNum >> cseq;
	}

};

inline std :: ostream & operator<< ( std :: ostream & os, const RAck & r ) {
	return os << r.responseNum << ' ' << r.cseq;
}

struct DialogId {
	ss :: string callId, localTag, remoteTag;
};

inline std :: ostream & operator<< ( std :: ostream & os, const DialogId & d ) {
	return os << d.callId << ':' << d.localTag << ':' << d.remoteTag;
}

DialogId makeDialogIdReceived ( const Request & r );
DialogId makeDialogIdReceived ( const Response & r );

inline bool operator!= ( const DialogId & d1, const DialogId & d2 ) {
	return d1.callId != d2.callId || d1.localTag != d2.localTag || d1.remoteTag != d2.remoteTag;
}

inline bool operator< ( const DialogId & d1, const DialogId & d2 ) {
	if ( int r = d1.callId.compare ( d2.callId ) )
		return r < 0;
	if ( int r = d1.localTag.compare ( d2.localTag ) )
		return r < 0;
	return d1.remoteTag < d2.remoteTag;
}

class Dialog : public Allocatable < __SS_ALLOCATOR > {
	class Impl;
	Impl * impl;
public:
	explicit Dialog ( const Request & r ); // prishel invite
	Dialog ( const Response & r, bool acceptsDtmfOverride ); // otvet na invite
	Dialog ( );
	~Dialog ( );
	Dialog ( const Dialog & d );
	Dialog & operator= ( const Dialog & d ) {
		Dialog ( d ).swap ( * this );
		return * this;
	}
	void swap ( Dialog & d ) {
		std :: swap ( impl, d.impl );
	}
	const DialogId & getId ( ) const;
	void targetRefresh ( const Response & r );
	bool requestReceived ( const Request & r );
	const URI & getRemoteTarget ( ) const;
	const URI & getRemoteUri ( ) const;
	const URI & getLocalUri ( ) const;
	int getLocalSeq ( ) const;
	void incLocalSeq ( );
	const RouteHeaderVector & getRouteSet ( ) const;
	bool isConfirmed ( ) const;
	void handleRecvInfo ( const ResponseMIMEInfo & m );
	bool getSupportsPackage ( ) const;
	bool getSupportsDtmf ( ) const;
	bool getAcceptsDtmf ( ) const;
	void handleAllow ( const ResponseMIMEInfo & m );
	bool getAllowsInfo ( ) const;
};

struct ClientTransactionId {
	ss :: string branch;
	Request :: Methods method;
	ClientTransactionId ( const ss :: string & b, Request :: Methods m ) : branch ( b ), method ( m ) { }
	explicit ClientTransactionId ( const Response & r ) : branch ( r.getMime ( ).getVia ( ).front ( ).getBranch ( ) ),
		method ( r.getMime ( ).getCseq ( ).method ) { }
	explicit ClientTransactionId ( const Request & r ) : branch ( r.getMime ( ).getVia ( ).front ( ).getBranch ( ) ),
		method ( r.getMethod ( ) ) { }
};

inline bool operator!= ( const ClientTransactionId & i1, const ClientTransactionId & i2 ) {
	return i1.branch != i1.branch || i1.method != i2.method;
}

inline bool operator< ( const ClientTransactionId & i1, const ClientTransactionId & i2 ) {
	if ( int r = i1.branch.compare ( i2.branch ) ) // vrode branch case-insensitive
		return r < 0;
	return i1.method < i2.method;
}

ss :: string methodName ( Request :: Methods m );

inline std :: ostream & operator<< ( std :: ostream & os, const ClientTransactionId & i ) {
	return os << i.branch << ':' << methodName ( i.method );
}

class OriginateHandler;

class ClientTransaction : public Allocatable < __SS_ALLOCATOR > {
public:
	virtual ~ClientTransaction ( ) { }
	virtual const ClientTransactionId & getId ( ) const = 0;
	virtual bool checkTimeout ( ) = 0;
	virtual bool responseReceived ( const Response & r ) = 0;
	virtual void detachHandler ( ) = 0;
	virtual OriginateHandler * getHandler ( ) const = 0;
	virtual const Request & getRequest ( ) const = 0;
	virtual const StringVector & getSavedPasswords ( ) const = 0;
	virtual void saveAuth ( const StringVector & passwords ) = 0;
};

class CoreClientTransactionHandler {
public:
	virtual void responseReceived ( const Response & r, const ClientTransaction * c ) const = 0;
	virtual void transactionTimedOut ( const ClientTransaction * c ) const = 0;
	virtual void transactionTransportError ( const ClientTransaction * c ) const = 0;
	virtual void transactionFinished ( const ClientTransaction * c ) const = 0;
	virtual ~CoreClientTransactionHandler ( ) { }
};

class InviteClientTransaction : public ClientTransaction {
	class Impl;
	Impl * impl;
	const ClientTransactionId & getId ( ) const;
	bool checkTimeout ( );
	bool responseReceived ( const Response & r );
	void detachHandler ( );
	OriginateHandler * getHandler ( ) const;
	const Request & getRequest ( ) const;
	const StringVector & getSavedPasswords ( ) const;
	void saveAuth ( const StringVector & passwords );
public:
	InviteClientTransaction ( const Request & r, const CoreClientTransactionHandler & c, OriginateHandler * u );
	~InviteClientTransaction ( );
	InviteClientTransaction ( const InviteClientTransaction & d );
	InviteClientTransaction & operator= ( const InviteClientTransaction & d ) {
		InviteClientTransaction ( d ).swap ( * this );
		return * this;
	}
	void swap ( InviteClientTransaction & d ) {
		std :: swap ( impl, d.impl );
	}
};

class NonInviteClientTransaction : public ClientTransaction {
	class Impl;
	Impl * impl;
	const ClientTransactionId & getId ( ) const;
	bool checkTimeout ( );
	bool responseReceived ( const Response & r );
	void detachHandler ( );
	OriginateHandler * getHandler ( ) const;
	const Request & getRequest ( ) const;
	const StringVector & getSavedPasswords ( ) const;
	void saveAuth ( const StringVector & passwords );
public:
	explicit NonInviteClientTransaction ( const Request & r, CoreClientTransactionHandler * c = 0, OriginateHandler * h = 0 );
	~NonInviteClientTransaction ( );
	NonInviteClientTransaction ( const NonInviteClientTransaction & d );
	NonInviteClientTransaction & operator= ( const NonInviteClientTransaction & d ) {
		NonInviteClientTransaction ( d ).swap ( * this );
		return * this;
	}
	void swap ( NonInviteClientTransaction & d ) {
		std :: swap ( impl, d.impl );
	}
};

struct ServerTransactionId { //tolko compliant poka
	ss :: string branch;
	ss :: string sentBy;
	Request :: Methods method;
	ServerTransactionId ( const ss :: string & b, const ss :: string & s, Request :: Methods m ) : branch ( b ),
		sentBy ( s ), method ( m ) { }
	explicit ServerTransactionId ( const Request & r ) : branch ( r.getMime ( ).getVia ( ).front ( ).getBranch ( ) ),
		sentBy ( r.getMime ( ).getVia ( ).front ( ).getHost ( ) ),
		method ( r.getMethod ( ) == Request :: mAck ? Request :: mInvite : r.getMethod ( ) ) { }
};

inline bool operator!= ( const ServerTransactionId & i1, const ServerTransactionId & i2 ) {
	return i1.branch != i2.branch || i1.sentBy != i2.sentBy || i1.method != i2.method;
}

inline bool operator< ( const ServerTransactionId & i1, const ServerTransactionId & i2 ) {
	if ( int r = i1.branch.compare ( i2.branch ) ) // vrode branch case-insensitive
		return r < 0;
	if ( int r = i1.sentBy.compare ( i2.sentBy ) ) // hostname case-insensitive
		return r < 0;
	return i1.method < i2.method;
}

inline std :: ostream & operator<< ( std :: ostream & os, const ServerTransactionId & i ) {
	return os << i.branch << ':' << i.sentBy << ':' << methodName ( i.method );
}

struct MergedTransactionId {
	ss :: string fromTag;
	ss :: string callId;
	CSeq seq;
	explicit MergedTransactionId ( const Request & r ) : fromTag ( r.getMime ( ).getFrom ( ).getTag ( ) ),
		callId ( r.getMime ( ).getCallId ( ) ), seq ( r.getMime ( ).getCseq ( ) ) { }
};

inline bool operator!= ( const MergedTransactionId & i1, const MergedTransactionId & i2 ) {
	return i1.fromTag != i2.fromTag || i1.callId != i2.callId || i1.seq != i2.seq;
}

inline bool operator< ( const MergedTransactionId & i1, const MergedTransactionId & i2 ) {
	if ( int r = i1.fromTag.compare ( i2.fromTag ) )
		return r < 0;
	if ( int r = i1.callId.compare ( i2.callId ) )
		return r < 0;
	return i1.seq < i2.seq;
}

class ServerTransaction : public Allocatable < __SS_ALLOCATOR > {
public:
	virtual ~ServerTransaction ( ) { }
	virtual const ServerTransactionId & getId ( ) const = 0;
	virtual const MergedTransactionId & getMergedId ( ) const = 0;
	virtual bool checkTimeout ( ) = 0;
	virtual bool responseSent ( const Response & r ) = 0;
	virtual bool requestReceived ( const Request & r ) = 0;
};

class CoreServerTransactionHandler {
public:
	virtual ~CoreServerTransactionHandler ( ) { }
	virtual void transactionTransportError ( ServerTransaction * i ) = 0;
};

class NonInviteServerTransaction : public ServerTransaction {
	class Impl;
	Impl * impl;
	const ServerTransactionId & getId ( ) const;
	const MergedTransactionId & getMergedId ( ) const;
	bool checkTimeout ( );
	bool responseSent ( const Response & r );
	bool requestReceived ( const Request & r );
public:
	explicit NonInviteServerTransaction ( const Request & r, CoreServerTransactionHandler * c = 0 );
	~NonInviteServerTransaction ( );
	NonInviteServerTransaction ( const NonInviteServerTransaction & d );
	NonInviteServerTransaction & operator= ( const NonInviteServerTransaction & d ) {
		NonInviteServerTransaction ( d ).swap ( * this );
		return * this;
	}
	void swap ( NonInviteServerTransaction & d ) {
		std :: swap ( impl, d.impl );
	}
};

class InviteServerTransaction : public ServerTransaction {
	class Impl;
	Impl * impl;
	const ServerTransactionId & getId ( ) const;
	const MergedTransactionId & getMergedId ( ) const;
	bool checkTimeout ( );
	bool responseSent ( const Response & r );
	bool requestReceived ( const Request & r );
public:
	explicit InviteServerTransaction ( const Request & r, CoreServerTransactionHandler * c = 0 );
	~InviteServerTransaction ( );
	InviteServerTransaction ( const InviteServerTransaction & d );
	InviteServerTransaction & operator= ( const InviteServerTransaction & d ) {
		InviteServerTransaction ( d ).swap ( * this );
		return * this;
	}
	void swap ( InviteServerTransaction & d ) {
		std :: swap ( impl, d.impl );
	}
	const Request & getRequest ( ) const;
	const Response & getResponse ( ) const;
	const DigestResponse * getSavedDigestResponse ( ) const;
	const ss :: string & getSavedPassword ( ) const;
	void saveAuth ( unsigned dIndex, const ss :: string & password );
};

ss :: string toHex ( const ss :: string & s );

}

#endif /*SIP2_HPP_*/
