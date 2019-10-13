#pragma implementation "sip.hpp"
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "ixcudpsocket.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include "sip.hpp"
#include <ptlib/svcproc.h>

#include "pointer.hpp" //for safeDel
#include <stdexcept>
#include "namespair.hpp"

#include <cstring>

namespace SIP  {

ss :: string trim ( const ss :: string & s ) {
	ss :: string t = s;
	while ( ! t.empty ( ) && ( t [ 0 ] == ' ' || t [ 0 ] == '\r' ) )
		t.erase ( 0, 1 );
	while ( ! t.empty ( ) && ( t [ t.size ( ) - 1 ] == ' ' || t [ t.size ( ) - 1 ] == '\r' ) )
		t.erase ( t.size ( ) - 1 );
	return t;
}

int atoi ( const ss :: string & s ) {
	return std :: atoi ( s.c_str ( ) );
}

int compareNoCase ( const ss :: string & s1, const ss :: string & s2 ) {
	ss :: string :: const_iterator p1 = s1.begin ( );
	ss :: string :: const_iterator p2 = s2.begin ( );
	while ( p1 != s1.end ( ) && p2 != s2.end ( ) ) {
		if ( std :: toupper ( * p1 ) != std :: toupper ( * p2 ) )
			return ( std :: toupper ( * p1 ) < std :: toupper ( * p2 ) ) ? -1 : 1;
		++ p1;
		++ p2;
	}
	int rez = ( s2.size ( ) == s1.size ( ) ) ? 0 : ( ( s1.size ( ) < s2.size ( ) )  ? -1 : 1 );
	return rez;
}

static ss :: string getConnectAddressString ( const TransportAddress & address ) {
	if ( address.getAddr ( ).empty ( ) )
		return "";
	return "IN IP4 " + address.getAddr ( );
}

static TransportAddress parseConnectAddress ( const ss :: string & s1, const ss :: string & s2,
	const ss :: string & s3 ) {
	if ( s1 == "IN" ) {
		if ( s2 == "IP4" )
			return s3;
		else {
			PTRACE ( 1, "SDP\tConnect address has invalid address type \"" << s2 << '\"' );
		}
	} else {
		PTRACE ( 1, "SDP\tConnect address has invalid network \"" << s1 << '\"' );
	}
	return TransportAddress ( );
}

static TransportAddress parseConnectAddress ( const ss :: string & s ) {
	ss :: istringstream is ( s );
	ss :: string s1, s2, s3;
	is >> s1 >> s2 >> s3;
	return parseConnectAddress ( s1, s2, s3 );
}

TransportAddress :: TransportAddress ( const ss :: string & s ) {
	ss :: string :: size_type pos = s.find ( '$' );
	if ( pos != ss :: string :: npos )
		proto = s.substr ( 0, pos ++ );
	else {
		proto = "tcp";
		pos = 0;
	}
	ss :: string :: size_type pos2 = s.find ( ':', pos );
	if ( pos2 != ss :: string :: npos )
		port = short ( atoi ( s.substr ( pos2 + 1 ) ) );
	else
		port = 0;
	addr = s.substr ( pos, pos2 - pos );
}

TransportAddress :: TransportAddress ( const ss :: string & ip, unsigned short p, const ss :: string & pr ) :
	proto ( pr ), addr ( ip ), port ( p ) {
	if ( 0 == port || 0xFFFF == port)
		PSYSTEMLOG ( Info, "TA::TA(ip, port, pr). ip = " << addr << ", port = 0, pr = " << proto );
}

void TransportAddress :: printOn ( std :: ostream & os ) const {
	os << proto << '$' << addr;
	if ( port )
		os << ':' << port;
}

ss :: string TransportAddress :: str ( ) const {
	ss :: ostringstream os;
	printOn ( os );
	return os.str ( );
}

const ss :: string & TransportAddress :: getAddr ( ) const {
	return addr;
}

void TransportAddress :: setAddr ( const ss :: string & a ) {
	addr = a;
}

unsigned short TransportAddress :: getPort ( ) const {
	return port;
}

void TransportAddress :: setPort ( unsigned short p ) {
	port = p;
}

void TransportAddress :: getIpAndPort ( PIPSocket :: Address & ip, unsigned short & p ) const {
	ip = addr.c_str ( );
	p = port;
	if ( 0 == port || 0xFFFF == port )
		PSYSTEMLOG ( Info, "TransportAddress :: getIpAndPort: addr = " << addr << ", port = " << port );
}

/*MediaFormat :: PayloadTypes*/ unsigned MediaFormat :: getPayloadType ( ) const {
	return payloadType;
}

ss :: string MediaFormat :: getPayloadTypePrintable ( ) const {
	if ( payloadType == T38 )
		return "t38";
	ss :: ostringstream os;
	os << payloadType;
	return os.str ( );
}

const ss :: string & MediaFormat :: getEncodingName ( ) const {
	return encodingName;
}

void MediaFormat :: setEncodingName ( const ss :: string & v ) {
	encodingName = v;
}

void MediaFormat :: setClockRate ( unsigned v ) {
	clockRate = v;
}

void MediaFormat :: setParameters ( const ss :: string & v ) {
	parameters = v;
}

MediaFormat & MediaFormat :: setPtime ( unsigned pt ) {
	ptime = pt;
	return * this;
}

unsigned MediaFormat :: getPtime ( ) const {
	return ptime;
}

static const ss :: string & defaultEncodingName ( unsigned pt ) {
	static NamesPair namePairs [ ] = {
		{ SIP :: MediaFormat :: PCMU, "PCMU" },
		{ SIP :: MediaFormat :: G726, "G726" },
		{ SIP :: MediaFormat :: GSM, "GSM" },
		{ SIP :: MediaFormat :: G7231, "G723" },
		{ SIP :: MediaFormat :: PCMA, "PCMA" },
		{ SIP :: MediaFormat :: G729, "G729" }
	};
	static const NamesMapType namesMap ( namePairs, namePairs + sizeof ( namePairs ) / sizeof ( * namePairs ) );
	NamesMapType :: const_iterator i = namesMap.find ( pt );
	if ( i == namesMap.end ( ) ) {
		static const ss :: string empty;
		return empty;
	}
	return i -> second;
}

MediaFormat :: MediaFormat ( unsigned pt, const ss :: string & _name, unsigned _clockRate,
	const ss :: string & _parms, const ss :: string & _fmtp ) : payloadType ( pt ),
	clockRate ( _clockRate ), encodingName ( _name ), parameters ( _parms ), fmtp ( _fmtp ), ptime ( 0 ) {
	if ( encodingName.empty ( ) )
		encodingName = defaultEncodingName ( payloadType );
}

MediaFormat & MediaFormat :: setFMTP ( const ss :: string & str ) {
	fmtp = str;
	return * this;
}

void MediaFormat :: decodeFMTP ( StringStringMap & m ) const {
	ss :: istringstream is ( fmtp );
	ss :: string s;
	while ( getline ( is, s, ',' ) ) {
		ss :: string :: size_type pos = s.find ( '=' );
		if ( pos == ss :: string :: npos ) {
			m.insert ( std :: make_pair ( trim ( s ), ss :: string ( ) ) );
			continue;
		}
		ss :: string k = trim ( s.substr ( 0, pos ) );
		ss :: string v = trim ( s.substr ( pos + 1 ) );
		m [ k ] = v;
	}
}

void MediaFormat :: encodeFMTP ( const StringStringMap & m ) {
	ss :: ostringstream os;
	bool first = true;
	for ( StringStringMap :: const_iterator i = m.begin ( ); i != m.end ( ); ++ i ) {
		if ( first )
			first = false;
		else
			os << ',';
		os << i -> first;
		if ( ! i -> second.empty ( ) )
			os << '=' <<  i -> second;
	}
	fmtp = os.str ( );
}

void MediaFormat :: addFMTP ( const ss :: string & key, const ss :: string & val ) {
	StringStringMap m;
	decodeFMTP ( m );
	m [ trim ( key ) ] = trim ( val );
	encodeFMTP ( m );
}

const ss :: string & MediaFormat :: getFMTP ( ) const {
	return fmtp;
}

ss :: string MediaFormat :: getFMTP ( const ss :: string & key ) const {
	StringStringMap m;
	decodeFMTP ( m );
	return m [ key ];
}

void MediaFormat :: printOn ( std :: ostream & os ) const {
	os << "a=rtpmap:" << int ( payloadType ) << ' ' << encodingName << '/' << clockRate;
	if ( ! parameters.empty ( ) )
		os << '/' << parameters;
	os << "\r\n";
	if ( ! fmtp.empty ( ) )
		os << "a=fmtp:" << int ( payloadType ) << ' ' << fmtp << "\r\n";
}

MediaDescription :: MediaType MediaDescription :: getMediaType ( ) const {
	return mediaType;
}

const MediaFormatVector & MediaDescription :: getMediaFormats ( ) const {
	return formats;
}

void MediaDescription :: setMediaFormats ( const MediaFormatVector & f ) {
	formats = f;
}

const TransportAddress & MediaDescription :: getTransportAddress ( ) const {
	return transportAddress;
}

void MediaDescription :: setTransportAddress ( const TransportAddress & t )
{
	transportAddress = t;
}

const ss :: string & MediaDescription :: getTransport ( ) const {
	return transport;
}

void MediaDescription :: setTransport ( const ss :: string & v ) {
	transport = v;
}

unsigned MediaDescription :: getPtime ( ) const {
	unsigned ptime = 0;
	for ( MediaFormatVector :: const_iterator i = formats.begin ( ); i != formats.end ( ); ++ i ) {
		unsigned pt = i -> getPtime ( );
		if ( ! pt )
			continue;
		if ( ! ptime )
			ptime = pt;
		else if ( pt != ptime  )
			return 0;
	}
	return ptime;
}

void MediaDescription :: setMaxptime ( unsigned mpt ) {
	maxptime = mpt;
}

unsigned MediaDescription :: getMaxptime ( ) const {
	return maxptime;
}

const ss :: string MediaDescription :: getAuthenticate() const
{
	return authenticate_;
}

void MediaDescription :: setAuthenticate ( const ss :: string & authenticate) {
	authenticate_ = authenticate;
}

MediaDescription :: MediaDescription ( const TransportAddress & address, MediaType _mediaType, TransportType _tt ) :
	transportAddress ( address ), maxptime ( 0 ), uTelephoneEventsPayloadType_( 0 ),
	mediaType ( _mediaType ), transportType ( _tt )  {

    switch ( mediaType ) {
		case mtAudio:
			media = "audio";
			break;
		case mtVideo:
			media = "video";
			break;
		case mtImage:
			media = "image";
		default:
			break;
	}
	switch ( _tt ) {
		case ttRtpAvp:
			transport = "RTP/AVP";
			break;
		case ttUdptl:
			transport = "udptl";
		default:
			break;
	}
}

void MediaDescription :: changeIPInAlt ( const SIP :: TransportAddress & newIP, ss :: string * alt ) {
	if ( alt == NULL )
		return;
	// alt:1 1 : 5E5BA07E 0000001F 193.108.123.139 7298
	ss :: string addr1, addr2, addr3, addrName, addrName2, ip, port;
	ss :: istringstream is ( * alt );
	is >> addr1 >> addr2 >> addr3 >> addrName >> addrName2 >> ip >> port;

	ss :: ostringstream os;
	os << addr1 << " " << addr2 << " " << addr3 << " " << addrName << " " << addrName2 << " " <<
		newIP.getAddr ( )  << " " << newIP.getPort ( );
	* alt = os.str ( );
}


bool MediaDescription :: decode ( const ss :: string & str ) {
	formats.clear ( );
	ss :: istringstream is ( str );

	is >> media;
	if ( media == "video" )
		mediaType = mtVideo;
	else if ( media == "audio" )
		mediaType = mtAudio;
	else if ( media == "image" )
		mediaType = mtImage;
	else {
		PTRACE ( 1, "SDP\tUnknown media type " << media );
		mediaType = mtUnknown;
	}

	ss :: string portStr;
	is >> portStr >> transport;
	// parse the port and port count
	ss :: string :: size_type pos = portStr.find ( '/' );
	if ( pos == ss :: string :: npos )
		portCount = 1;
	else {
		PTRACE ( 1, "SDP\tMedia header contains port count - " << portStr );
		portCount = short ( atoi ( portStr.substr ( pos + 1 ) ) );
		portStr = portStr.substr ( 0, pos );
	}
	unsigned short port = short ( atoi ( portStr ) );

//    PSYSTEMLOG(Info, "MediaDescription :: decode: port = " << port );

	if ( transport == "RTP/AVP" )
		transportType = ttRtpAvp;
	else if ( transport == "udptl" )
		transportType = ttUdptl;
	else {
		PTRACE ( 1, "SDP\tUnknown transport " << transport );
		return false;
	}
	transportAddress.setPort ( port );
	if ( transportAddress.getAddr ( ).empty ( ) )
		transportAddress.setAddr ( "127.0.0.1" );

    PSYSTEMLOG(Info, "MediaDescription :: decode: " << transportAddress.str() );

	if ( transportType == ttUdptl ) {
		ss :: string t;
		while ( is >> t ) {
			if ( t == "t38" )
				formats.push_back ( MediaFormat ( MediaFormat :: T38 ) );
			else {
				PSYSTEMLOG ( Error, "unknown udptl payload " << t );
				return false;
			}
		}
		return ! formats.empty ( );
	}
	// create the format list
	unsigned t = 0;
	while ( is >> t )
		formats.push_back ( MediaFormat ( MediaFormat :: PayloadTypes ( t ) ) );
	return ! formats.empty ( );
}

void MediaDescription :: setAttribute ( const ss :: string & ostr ) {
	// get the attribute type

//    PSYSTEMLOG(Info, "MediaDescription :: setAttribute = " << ostr );
	ss :: string :: size_type pos = ostr.find ( ':' );
	if ( pos == ss :: string :: npos ) {
		unsupportedAttributes [ ostr ] = "";
		PSYSTEMLOG ( Info, "unsupportedAttributes [" << ostr << "] = ''." );
		return;
	}
	ss :: string attr = ostr.substr ( 0, pos );
	ss :: string str = ostr.substr ( pos + 1 );
	if ( attr == "ptime" ) {
		unsigned ptime = atoi ( str );
		for ( MediaFormatVector :: iterator i = formats.begin ( ); i != formats.end ( ); ++ i )
			formats.replace ( i, MediaFormat ( * i ).setPtime ( ptime ) );
		return;
	}
	if ( attr == "maxptime" ) {
		maxptime = atoi ( str );
		return;
	}

	if ( attr == "P-Asserted-Identity" || attr == "p-asserted-identity") {

		pAssertedId_ = str;
		return;
	}

	if ( attr == "WWW-Authenticate" ) {

		authenticate_ = str;
		return;
	}

	if ( attr == "PROXY-Authenticate" ) {

		authenticate_ = str;
		return;
	}

	if ( attr == "alt" ) {
		alt_ = str;
		//changeIPInAlt( transportAddress, &alt_);
		return;
	}

//	bool sendReqv = false;
//
//	if( attr == "sendrecv" || attr == "sendonly" || attr == "recvonly" ) {
//		PSYSTEMLOG ( Info, "sendReqv = true.");
//		sendReqv = true;
//	}

	if ( attr != "rtpmap" && attr != "fmtp" ) {
		unsupportedAttributes [ attr ] = trim ( str );
		return; // sendReqv;
	}
	// extract the RTP payload type
	pos = str.find ( ' ' );
	if ( pos == ss :: string :: npos ) {
		PSYSTEMLOG ( Error, "Malformed media attribute " << ostr );
		return;
	}

	unsigned codecIndex = atoi ( str.substr ( 0, pos ) );

	typedef MediaFormatVector :: index < payload > :: type FormatByPayload;
	FormatByPayload & formatsByPayload = formats.get < payload > ( );
	FormatByPayload :: iterator fmt = formatsByPayload.find ( codecIndex );
	if ( fmt == formatsByPayload.end ( ) ) {
		PSYSTEMLOG ( Error, "Media attribute " << attr << " found for unknown RTP type " << codecIndex );
		return;
	}

	// extract the attribute argument
	str = trim ( str.substr ( pos + 1 ) );

	// handle rtpmap attribute
	if ( attr == "rtpmap" ) {
		//PSYSTEMLOG ( Info, "attr == 'rtpmap' " << str);
		ss :: string :: size_type s1 = str.find ( '/' );
		if ( s1 == ss :: string :: npos ) {
			PSYSTEMLOG ( Error, "Malformed rtpmap attribute for " << codecIndex );
			return;
		}
		MediaFormat format = * fmt;
		ss :: string payloadName = str.substr ( 0, s1 );
		format.setEncodingName ( payloadName );
		//PSYSTEMLOG ( Info, "Payload index " << codecIndex << ", payloadName = " << payloadName );
		if(payloadName == "telephone-event")
		{
			uTelephoneEventsPayloadType_ = codecIndex;
//			PSYSTEMLOG(Info, "MediaDescription: decode. telephone-event.");
		}
		format.setClockRate ( atoi ( str.substr ( ++ s1 ) ) );
		ss :: string :: size_type s2 = str.find ( '/', s1 );
		if ( s2 != ss :: string :: npos )
			format.setParameters ( str.substr ( s2 + 1 ) );
		formatsByPayload.replace ( fmt, format );
		return;
	}

	// handle fmtp attributes
	if ( attr == "fmtp" ) {
		MediaFormat format = * fmt;
		format.setFMTP ( str );
		formatsByPayload.replace ( fmt, format );
		return;
	}
	// unknown attriutes
	PSYSTEMLOG ( Error, "Unknown media attribute " << ostr );
}

void MediaDescription :: printOn ( std :: ostream & os ) const {
	// output media header
	os << "m=" << media << " " << transportAddress.getPort ( ) << " " << transport;
	// output RTP payload types
	for ( MediaFormatVector :: const_iterator i = formats.begin ( ); i != formats.end ( ); ++ i )
	{
        os << ' ' << i -> getPayloadTypePrintable ( );
    }

    os << "\r\n"
		"c=" << getConnectAddressString ( transportAddress ) << "\r\n";
	// output attributes for each payload type
	if ( ! alt_.empty ( ) ) {
		//changeIPInAlt(transportAddress, &alt_);
		os << "a=alt:" << alt_ << "\r\n";
	}
	for ( MediaFormatVector :: const_iterator i = formats.begin ( ); i != formats.end ( ); ++ i ) {
		const MediaFormat & format = * i;
		if ( format.getPayloadType ( ) > MediaFormat :: LastKnownPayloadType || ! format.getEncodingName ( ).empty ( ) )
		{	os << format;
        }
	}

	if ( int ptime = getPtime ( ) )
		os << "a=ptime:" << ptime << "\r\n";
	if ( maxptime )
		os << "a=maxptime:" << maxptime << "\r\n";
	for ( StringStringMap :: const_iterator i = unsupportedAttributes.begin ( );
		i != unsupportedAttributes.end ( ); ++ i ) {
		if ( i -> second.empty ( ) )
			os << "a=" << i -> first << "\r\n";
		else
			os << "a=" << i -> first << ':' << i -> second << "\r\n";
	}
}

unsigned MediaDescription :: getTelephoneEventsPayloadType() const
{
	return uTelephoneEventsPayloadType_;
}

void MediaDescription :: setTelephoneEventsPayloadType( unsigned payload )
{
	uTelephoneEventsPayloadType_ = payload;
}

void MediaDescription :: addMediaFormat ( const MediaFormat & sdpMediaFormat ) {
	formats.push_back ( sdpMediaFormat );
}

void SessionDescription :: setSessionName ( const ss :: string & v ) {
	sessionName = v;
}

const ss :: string & SessionDescription :: getSessionName ( ) const {
	return sessionName;
}

void SessionDescription :: setUserName ( const ss :: string & v ) {
	ownerUsername = v;
}

const ss :: string & SessionDescription :: getUserName ( ) const {
	return ownerUsername;
}

const MediaDescriptionVector & SessionDescription :: getMediaDescriptions ( ) const {
	return mediaDescriptions;
}

void SessionDescription :: addMediaDescription ( const MediaDescription & md ) {
	mediaDescriptions.push_back ( md );
}

const TransportAddress & SessionDescription :: getDefaultConnectAddress ( ) const {
	return defaultConnectAddress;
}

void SessionDescription :: setDefaultConnectAddress ( const TransportAddress & address ) {
	defaultConnectAddress = address;

    //setMediaTransportAddress ();
}

void SessionDescription :: setOwnerAddress ( const TransportAddress & o ) {
	ownerAddress = o;
}

TransportAddress SessionDescription :: getOwnerAddress ( ) const {
	return ownerAddress;
}

#define SIP_DEFAULT_SESSION_NAME  "SS SIP Session"

SessionDescription :: SessionDescription ( ) :
//	sendReqv_(false),
	sessionName ( SIP_DEFAULT_SESSION_NAME ), ownerUsername ( "-" ),
	protocolVersion ( 0 ),
	ownerSessionId ( int ( PTime ( ).GetTimeInSeconds ( ) ) ),
	ownerVersion ( ownerSessionId ),
	uTelephoneEventsPayloadType_( 0 ) { }

SessionDescription :: SessionDescription ( std :: istream & is ) :
//	sendReqv_(false),
	sessionName ( SIP_DEFAULT_SESSION_NAME ), ownerUsername ( "-" ),
	protocolVersion ( 0 ),
	ownerSessionId ( int ( PTime ( ).GetTimeInSeconds ( ) ) ),
	ownerVersion ( ownerSessionId ) {
	if(false == readFrom ( is ))
	{
		throw std :: runtime_error("Do not create SessionDescription. Wrong port number.");
	}
//	PSYSTEMLOG ( Info, "defaultConnectAddres " << defaultConnectAddress << ", ownerAddress "<< ownerAddress);
////	if(true == sendReqv_)
//		defaultConnectAddress = ownerAddress;
}

void SessionDescription :: printOn ( std :: ostream & os ) const{
	// encode mandatory session information
	os << "v=" << protocolVersion << "\r\n"
		"o=" << ownerUsername << ' ' << ownerSessionId << ' ' << ownerVersion << ' '
		<< getConnectAddressString ( ownerAddress ) << "\r\n"
		"s=" << sessionName << "\r\n"
		"c=" << getConnectAddressString ( defaultConnectAddress ) << "\r\n"
//		"c=" << getConnectAddressString ( ownerAddress ) << "\r\n"
		"t=" << "0 0" << "\r\n";

	// encode media session information
	for ( unsigned i = 0; i < mediaDescriptions.size ( ); i ++ )
	{
        os << mediaDescriptions [ i ];
    }
}

ss :: string SessionDescription :: str ( ) const {
	ss :: ostringstream os;
    printOn ( os );
	return os.str ( );
}

bool SessionDescription :: readFrom ( std :: istream & is )
{
	mediaDescriptions.clear ( );
	// break string into lines
	ss :: string line;
	while ( getline ( is, line ) ) {
		if ( trim  ( line ).empty ( ) )
			return true;
		ss :: string :: size_type pos = line.find ( '=' );
		if ( pos == ss :: string :: npos ) {
			PTRACE ( 1, "SDP\tline w/o =: " << line );
			continue;
		}
		ss :: string key = trim ( line.substr ( 0, pos) );
		ss :: string value = trim ( line.substr ( pos + 1 ) );

		PSYSTEMLOG ( Info, "SessionDescription::readFrom: key='" << key << "', value='"<< value << "'" );
		if ( key.size ( ) != 1 ) {
			PTRACE ( 1, "SDP\tkey size isnt 1: " << key );
			continue;
		}

		// media name and transport address (mandatory)
		if ( key [ 0 ] == 'm' ) {
			MediaDescription currentMedia ( defaultConnectAddress );
//			MediaDescription currentMedia ( ownerAddress );
			if ( currentMedia.decode ( value ) ) {
				if(currentMedia.getTransportAddress ().getPort() == 0)
				{
					PSYSTEMLOG(Error, "Port number is zero.");
					return false;
				}
				addMediaDescription ( currentMedia );
				PTRACE ( 2, "SDP\tAdding media session with " << currentMedia.getMediaFormats ( ).size ( )
					<< " formats, port = " << currentMedia.getTransportAddress ().getPort() );
			}
		}

		/////////////////////////////////
		//
		// Session description
		//
		/////////////////////////////////

		else if ( mediaDescriptions.empty ( ) ) {
			switch ( key [ 0 ] ) {
				case 'v': // protocol version (mandatory)
					protocolVersion = atoi ( value );
					break;
				case 'o': // owner/creator and session identifier (mandatory)
					parseOwner ( value );
					break;
				case 's': // session name (mandatory)
					sessionName = value;
					break;
				case 'c': // connection information - not required if included in all media
					defaultConnectAddress = parseConnectAddress ( value );
					break;
				case 't': // time the session is active (mandatory)
					break;
				case 'i': // session information
				case 'u': // URI of description
				case 'e': // email address
				case 'p': // phone number
				case 'b': // bandwidth information
				case 'z': // time zone adjustments
				case 'k': // encryption key
				case 'a': // zero or more session attribute lines
				case 'r': // zero or more repeat times
				default:
					PTRACE ( 1, "SDP\tUnknown session information key " << key [ 0 ] );
			}
		}

		/////////////////////////////////
		//
		// media information
		//
		/////////////////////////////////

		else {
			switch ( key [ 0 ] ) {
				case 'c': { // connection information - optional if included at session-level
					TransportAddress addr = parseConnectAddress ( value );
					addr.setPort ( mediaDescriptions.back ( ).getTransportAddress ( ).getPort ( ) );
					mediaDescriptions.back ( ).setTransportAddress ( addr );
					break;
				} case 'a': // zero or more media attribute lines
					mediaDescriptions.back ( ).setAttribute ( value );
					break;
				case 'i': // media title
				case 'b': // bandwidth information
				case 'k': // encryption key
				default:
					PTRACE ( 1, "SDP\tUnknown mediainformation key " << key [ 0 ] );
			}
		}
	}
    //setMediaTransportAddress ();
	return true;
}

bool SessionDescription :: decode ( const ss :: string & str ) {
	ss :: istringstream is ( str );
	return readFrom ( is );
}

unsigned SessionDescription :: getTelephoneEventsPayloadType() const
{
//	for ( unsigned i = 0; i < mediaDescriptions.size ( ); i ++ )
	{
		return getMediaDescription ( SIP :: MediaDescription :: mtAudio )->getTelephoneEventsPayloadType();
/*
		if( 0 != mediaDescriptions [ i ].getTelephoneEventsPayloadType() )
			return mediaDescriptions [ i ].getTelephoneEventsPayloadType();
*/
	}

	return 0;
//	return uTelephoneEventsPayloadType_;
}

void SessionDescription :: setMediaTransportAddress ()
{
    MediaDescription* media = getMediaDescription ( SIP :: MediaDescription :: mtAudio );
    if(media)
    {
        TransportAddress ta = media->getTransportAddress();
//        TransportAddress ta1 = media->getTransportAddress();
        ta.setAddr(defaultConnectAddress.getAddr());
        media->setTransportAddress(ta);
//        PSYSTEMLOG(Info, "parseOvner: defaultConnectAddress: " << defaultConnectAddress << "; " << ta << "; " << ta1);
    }

}

void SessionDescription :: parseOwner ( const ss :: string & str ) {
	ss :: string addr1, addr2, addr3;
	ss :: istringstream is ( str );
	is >> ownerUsername >> ownerSessionId >> ownerVersion >> addr1 >> addr2 >> addr3;
	ownerAddress = defaultConnectAddress = parseConnectAddress ( addr1, addr2, addr3 );

    //setMediaTransportAddress ();
}

const MediaDescription * SessionDescription :: getMediaDescription ( MediaDescription :: MediaType rtpMediaType ) const
{
	// look for matching media type
	for ( unsigned i = 0; i < mediaDescriptions.size ( ); i ++ ) {
		if ( mediaDescriptions [ i ].getMediaType ( ) == rtpMediaType )
			return & mediaDescriptions [ i ];
	}
	return 0;
}

MediaDescription * SessionDescription :: getMediaDescription ( MediaDescription :: MediaType rtpMediaType ) {
	// look for matching media type
//	PSYSTEMLOG(Info, "SessionDescription::getMediaDescription: count=" << mediaDescriptions.size() << ", type=" << rtpMediaType << "(" << SIP :: MediaDescription :: mtAudio << ")" );
	for ( unsigned i = 0; i < mediaDescriptions.size ( ); i ++ ) {
		if ( mediaDescriptions [ i ].getMediaType ( ) == rtpMediaType )
			return & mediaDescriptions [ i ];
	}
	return 0;
}

URL :: URL ( ) : scheme ( "sip" ), port ( 0 ), relativePath ( false ) { }

URL :: URL ( const ss :: string & s ) {
	parse ( s );
}

URL & URL :: operator= ( const URL & u ) {
	scheme = u.scheme;
	username = u.username;
	password = u.password;
	hostname = u.hostname;
	port = u.port;
	relativePath = u.relativePath;
	pathStr = u.pathStr;
	path = u.path;
	paramVars = u.paramVars;
	fragment = u.fragment;
	queryVars = u.queryVars;
	sipParamVars = u.sipParamVars;
	displayName = u.displayName;
	if ( 0 == port || 0xFFFF == port )
		PSYSTEMLOG ( Info, "URL: port = " << port );
	return * this;
}

static void splitVars ( const ss :: string & str, StringStringMap & vars,
	char sep1, char sep2 ) {
	ss :: string :: size_type sep1prev = 0;
	do {
		ss :: string :: size_type sep1next = str.find ( sep1, sep1prev );
		if ( sep1next == ss :: string :: npos )
			sep1next --; // Implicit assumption string is not a couple of gigabytes long ...
		ss :: string :: size_type sep2pos = str.find ( sep2, sep1prev );
		if ( sep2pos > sep1next )
			sep2pos = sep1next;

		ss :: string key = URL :: untranslateString ( str.substr ( sep1prev, sep2pos - sep1prev ), URL :: ttQuery );
		if ( ! key.empty ( ) ) {
			ss :: string data;
			if ( sep2pos < str.size ( ) )
				data = URL :: untranslateString ( str.substr ( sep2pos + 1, sep1next - sep2pos - 1 ), URL :: ttQuery );
			if ( vars.count ( key ) )
				vars [ key ] = vars [ key ] + ',' + data;
			else
				vars [ key ] = data;
		}
		sep1prev = sep1next + 1;
	} while ( sep1prev != ss :: string :: npos );
}

void URL :: parse ( const ss :: string & s ) {
	ss :: string s1 = s;
	scheme = "sip";
	username.clear ( );
	password.clear ( );
	hostname.clear ( );
	port = 0;
	relativePath = false;
	pathStr.clear ( );
	path.clear ( );
	paramVars.clear ( );
	fragment.clear ( );
	queryVars.clear ( );
	sipParamVars.clear ( );
	displayName.clear ( );

	ss :: string :: size_type start = s1.rfind ( '<' );
	ss :: string :: size_type end = s1.rfind ( '>' );

	// see if URL is just a URI or it contains a display address as well
	if ( start == ss :: string :: npos || end == ss :: string :: npos ) {
		// may be URI is without < > (for example sip:someone@somewhere;tag=xxxx )
		start = s1.rfind ( "sip:" );
		if ( start == ss :: string :: npos ) {
			start = 0;
			end = s1.size ( );
		} else {
			end = s1.find ( ';', start );
			if ( end == ss :: string :: npos ) {
				end = s1.size ( );
				s1+= " ";
			}
			s1.insert ( start, "<" );
			s1.insert ( ++ end, ">" );
			start ++;
		}
	} else
		start ++;
	ss :: string url = trim ( s1.substr ( start, end - start ) );
	if ( ! url.empty ( ) && isalpha ( url [ 0 ] ) ) {
		ss :: string :: size_type pos = url.find_first_of ( "=;/#?:" );
		if ( pos != ss :: string :: npos && url [ pos ] == ':' ) {
			scheme = url.substr ( 0, pos );
			url.erase ( 0, pos + 1 );
		}
	}
	if ( scheme != "sip" )
		return;

	ss :: string :: size_type pos = url.find_first_of ( "/;?" );
	ss :: string uphp = url.substr ( 0, pos );
	url.erase ( 0, pos );

	// extract username and password
	ss :: string :: size_type pos2 = uphp.find ( '@' );
	if ( pos2 != ss :: string :: npos && pos2 > 0 ) {
		ss :: string :: size_type pos3 = uphp.find ( ':' );
		// if no password...
		if ( pos3 > pos2 )
			username = untranslateString ( uphp.substr ( 0, pos2 ), ttLogin );
		else {
			username = untranslateString ( uphp.substr ( 0, pos3 ), ttLogin );
			password = untranslateString ( uphp.substr ( pos3 + 1, pos2 - pos3 - 1 ), ttLogin );
		}
		uphp.erase ( 0, pos2 + 1 );
	}

	// determine if the URL has a port number
	pos = uphp.find ( ':' );
	if ( pos == ss :: string :: npos ) {
		hostname = untranslateString ( uphp, ttLogin );
		port = 5060;
	} else {
		hostname = untranslateString ( uphp.substr ( 0, pos ), ttLogin );
		port = short ( atoi ( uphp.substr ( pos + 1 ) ) );
		if ( 0 == port || 0xFFFF == port)
			PSYSTEMLOG ( Info, "URL::parse: port = " << port );
	}
	if ( hostname.empty ( ) )
		hostname = PIPSocket :: GetHostName ( );


	// chop off any trailing parameters
	pos = url.find ( ';' );
	if ( pos != ss :: string :: npos ) {
		splitVars ( url.substr ( pos + 1 ), paramVars, ';', '=' );
		url.erase ( pos );
	}


	// if the rest of the URL isn't a path, then we are finished!
	pathStr = untranslateString ( url, ttPath );

	if ( start == 0 )
		return;
	pos = s1.find ( ';', end + 1 );
	if ( pos != ss :: string :: npos )
		splitVars ( s1.substr ( pos + 1 ), sipParamVars, ';', '=' );

	// extract the display address
	ss :: string :: size_type oldstart = start;
	oldstart --;
	end = s1.rfind ( '\"', start - 1 );
	start = s1.rfind ( '\"', end - 1 );
	if ( start == ss :: string :: npos && end == ss :: string :: npos )
		displayName = trim ( s1.substr ( 0, oldstart ) );
	else if ( start != ss :: string :: npos && end != ss :: string :: npos && end > start ) {
		// No trim quotes off
		displayName = s1.substr ( start + 1, end - start - 1 );
		while ( ( start = displayName.find ( '\\' ) ) != ss :: string :: npos )
			displayName.erase ( start, 1 );
	}
}

static void printParams ( std :: ostream & os, const StringStringMap & vars ) {
	for ( StringStringMap :: const_iterator i = vars.begin ( );
		i != vars.end ( ); ++ i ) {
		os << ';' << URL :: translateString ( i -> first, URL :: ttQuery );
		if ( ! i -> second.empty ( ) )
			os << '=' << URL :: translateString ( i -> second, URL :: ttQuery );
	}
}

void URL :: printOn ( std :: ostream & os ) const {
	if ( ! displayName.empty ( ) )
		os << '\"' << displayName << "\" ";
	os << '<';
	os << scheme << ':';
	if ( ! username.empty ( ) ) {
		os << translateString ( username, ttLogin );
		if ( ! password.empty ( ) )
			os << ':' << translateString ( password, ttLogin );
		os << '@';
	}
	os << hostname;
	if ( port != 5060 )
		os << ':' << port;
	os << translateString ( pathStr, ttPath );
	printParams ( os, paramVars );
	os << '>';
	printParams ( os, sipParamVars );
}

ss :: string URL :: str ( ) const {
	ss :: ostringstream os;
	printOn ( os );
	return os.str ( );
}

ss :: string URL :: hostPort ( ) const {
	ss :: ostringstream os;
	os << hostname;
	if ( port != 5060 )
		os << ':' << port;
	return os.str ( );
}

ss :: string URL :: shortForm ( ) const {
	ss :: ostringstream os;
	os << scheme << ':';
	if ( ! username.empty ( ) ) {
		os << translateString ( username, ttLogin );
		if ( ! password.empty ( ) )
			os << ':' << translateString ( password, ttLogin );
		os << '@';
	}
	os << hostPort ( );
	return os.str ( );
}

ss :: string URL :: bracketShortForm ( ) const {
	ss :: ostringstream os;
	os << '<' << shortForm ( ) << '>';
	return os.str ( );
}

ss :: string URL :: translateString ( const ss :: string & str, TranslationType type ) {
	ss :: string xlat = str;
	ss :: string safeChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$-_.!*'(),";
	switch ( type ) {
		case ttLogin:
			//safeChars += "+;?&=#";
			safeChars += "+;?&=";
			break;
		case ttPath:
			safeChars += "+:@&=";
			break;
		case ttQuery:
			safeChars += ":@";
	        break;
        case ttURL:
            safeChars += "+;?&=:@< >%\"";
	        break;
	}
	ss :: string :: size_type pos = 0;
	while ( ( pos = xlat.find_first_not_of ( safeChars, pos ) ) != ss :: string :: npos ) {
		ss :: string t = xlat.substr ( 0, pos );
		t += '%';
		char buf [ 3 ];
		std :: sprintf ( buf, "%02X", ( unsigned char ) xlat [ pos ] );
		t += buf;
		t += xlat.substr ( pos + 1 );
		xlat = t;
		pos += 3;
	}
	if ( type == ttQuery ) {
		ss :: string :: size_type space = 0;
		while ( ( space = xlat.find ( ' ', space ) ) != ss :: string :: npos )
			xlat [ space ++ ] = '+';
	}
	return xlat;
}

ss :: string URL :: untranslateString ( const ss :: string & str, TranslationType type ) {
	ss :: string xlat = str;
	ss :: string :: size_type pos = 0;
	if ( type == ttQuery ) {
		while ( ( pos = xlat.find ( '+', pos ) ) != ss :: string :: npos )
			xlat [ pos ++ ] = ' ';
		pos = 0;
	}
	while ( ( pos = xlat.find ( '%', pos ) ) != ss :: string :: npos && pos + 2 < xlat.size ( ) ) {
		int digit1 = xlat [ pos + 1 ];
		int digit2 = xlat [ pos + 2 ];
		if ( std :: isxdigit ( digit1 ) && std :: isxdigit ( digit2 ) ) {
			xlat [ pos ] = char ( ( std :: isdigit ( digit2 ) ? ( digit2 - '0' ) :
				( std :: toupper ( digit2 ) - 'A' + 10 ) ) + ( ( std :: isdigit ( digit1 ) ?
				( digit1 - '0' ) : ( std :: toupper ( digit1 ) - 'A' + 10 ) ) << 4 ) );
			xlat.erase ( pos + 1, 2 );
		}
		pos ++;
	}
	return xlat;
}

const ss :: string & URL :: getHostName ( ) const {
	return hostname;
}

const ss :: string & URL :: getUserName ( ) const {
	return username;
}

unsigned short URL :: getPort ( ) const {
	if ( port == 0 || 0xFFFF == port )
		PSYSTEMLOG ( Error, "URL :: getPort: port = " << port );
	return port;
}

void URL :: setHostName ( const ss :: string & s ) {
	hostname = s;
}

void URL :: setUserName ( const ss :: string & s ) {
	username = s;
}

void URL :: setPort ( unsigned short p ) {
	port = p;
	if ( port == 0 || 0xFFFF == port)
		PSYSTEMLOG(Error, "URL :: setPort: port = " << port);
}

void URL :: setDisplayName ( const ss :: string & s ) {
	displayName = s;
}

void URL :: setSipParam ( const ss :: string & name, const ss :: string & val ) {
	sipParamVars [ name ] = val;
}

const ss :: string URL :: getSipParam ( const ss :: string & name ) const {
	ss :: string value;
	StringStringMap :: const_iterator i = sipParamVars.find ( name );
	if ( i == sipParamVars.end ( ) )
		return "";
	return i -> second;
}

//MIMEInfo :: MapCompactToLongForm compactFormMap_;
//MIMEInfo :: MapCompactToLongForm longFormMap_;

MIMEInfo :: MIMEInfo ( bool isRecordRouteRequired, bool _compactForm ) :
	compactForm ( _compactForm ),
	isRecordRouteRequired_(isRecordRouteRequired),
	isPrivecyHeader_(false)
{
	fillFormMap();
}

MIMEInfo :: MIMEInfo ( bool isRecordRouteRequired, const MIMEInfo & m ) :
	values ( m.values ), compactForm ( m.compactForm ),
	isRecordRouteRequired_(isRecordRouteRequired),
	isPrivecyHeader_(false)
{
	fillFormMap();
}

void MIMEInfo :: fillFormMap() {
	// TODO: make it global
	if(compactFormMap_.empty())
	{
		compactFormMap_.insert(make_pair(ss :: string("c"), ss :: string("Content-Type")));
		compactFormMap_.insert(make_pair(ss :: string("e"), ss :: string("Content-Encoding")));
		compactFormMap_.insert(make_pair(ss :: string("f"), ss :: string("From")));
		compactFormMap_.insert(make_pair(ss :: string("i"), ss :: string("Call-ID")));
		compactFormMap_.insert(make_pair(ss :: string("m"), ss :: string("Contact")));
		compactFormMap_.insert(make_pair(ss :: string("l"), ss :: string("Content-Length")));
		compactFormMap_.insert(make_pair(ss :: string("s"), ss :: string("Subject")));
		compactFormMap_.insert(make_pair(ss :: string("t"), ss :: string("To")));
		compactFormMap_.insert(make_pair(ss :: string("v"), ss :: string("Via")));
	}

	if(longFormMap_.empty())
	{
		longFormMap_.insert(make_pair(ss :: string("Content-Type"), ss :: string("c")));
		longFormMap_.insert(make_pair(ss :: string("Content-Encoding"), ss :: string("e")));
		longFormMap_.insert(make_pair(ss :: string("From"), ss :: string("f")));
		longFormMap_.insert(make_pair(ss :: string("Call-ID"), ss :: string("i")));
		longFormMap_.insert(make_pair(ss :: string("Contact"), ss :: string("m")));
		longFormMap_.insert(make_pair(ss :: string("Content-Length"), ss :: string("l")));
		longFormMap_.insert(make_pair(ss :: string("Subject"), ss :: string("s")));
		longFormMap_.insert(make_pair(ss :: string("To"), ss :: string("t")));
		longFormMap_.insert(make_pair(ss :: string("Via"), ss :: string("v")));
	}
}


bool MIMEInfo :: readFrom ( std :: istream & is ) {
//    PSYSTEMLOG(Info, "MIMEInfo :: readFrom" );
	values.clear ( );
	ss :: string line;
	while ( getline ( is, line ) ) {
		if ( trim ( line ).empty ( ) )
		{
            return true;
		}
		if ( ! addMIME ( line ) )
			break;
	}

//	updateRecordRoute();
	return false;
}

bool MIMEInfo :: write ( std :: ostream & os ) const {
PSYSTEMLOG(Info, "MIMEInfo :: write");
	for ( values_t :: const_iterator i = values.begin ( ); i != values.end ( ); ++ i ) {
		ss :: string name = i -> first + ": ";
		const ss :: string & value = i -> second;
		PSYSTEMLOG(Info, "MIMEInfo :: write: " << name << " - " << value);
		ss :: string :: size_type p1 = value.find_first_not_of ( "\r\n" ), p2;
		while ( ( p2 = value.find_first_of ( "\r\n", p1 ) ) != ss :: string :: npos ) {
			os << name << value.substr ( p1, p2 - p1 ) << "\r\n";
			p1 = value.find_first_not_of ( "\r\n", p2 + 1 );
		}
		if ( p1 < value.size ( ) )
			os << name << value.substr ( p1 ) << "\r\n";
		if ( ! os )
			return false;
	}

	if(isPrivecyHeader_)
	{
		os << "Privacy: id" << "\r\n";
	}
	printRecordRoute( os );

PSYSTEMLOG(Info, "MIMEInfo :: write: \n" << os );
	return os << "\r\n";
}

void MIMEInfo :: setForm ( bool v ) {
	compactForm = v;
}

unsigned MIMEInfo :: getCSeqIndex ( ) const {
	return atoi ( getCSeq ( ) );
}

int MIMEInfo :: getMaxForwards ( ) const {
	ss :: string t = get ( "Max-Forwards" );
	if ( t.empty ( ) )
		t = "70";
	return atoi ( t );
}

void MIMEInfo :: setMaxForwards ( int v ) {
	ss :: ostringstream os;
	os << v;
	set ( "Max-Forwards", os.str ( ) );
}

bool MIMEInfo :: updateMaxForwards ( const MIMEInfo & m ) {
	int a = m.getMaxForwards ( );
	setMaxForwards ( -- a );
	return a >= 0;
}

unsigned MIMEInfo :: getContentLength ( ) const {
	ss :: string len = getFullOrCompact ( "Content-Length", 'l' );
	return atoi ( len );
}

ss :: string MIMEInfo :: getContentType ( ) const {
	return getFullOrCompact ( "Content-Type", 'c' );
}

void MIMEInfo :: setContentType ( const ss :: string & v ) {
	set ( getFormString("Content-Type"), v );
}

ss :: string MIMEInfo :: getContentEncoding ( ) const {
	return getFullOrCompact ( "Content-Encoding", 'e' );
}

void MIMEInfo :: setContentEncoding ( const ss :: string & v ) {
	set ( getFormString("Content-Encoding"), v );
}

ss :: string MIMEInfo :: getFrom ( ) const {
	return getFullOrCompact ( "From", 'f' );
}

void MIMEInfo :: setFrom (const ss :: string & v ) {
	set ( getFormString("From"), v );
}

ss :: string MIMEInfo :: getCallID ( ) const {
	return getFullOrCompact ( "Call-ID", 'i' );
}

void MIMEInfo :: setCallID ( const ss :: string & v ) {
	set ( getFormString("Call-ID"), v );
}

ss :: string MIMEInfo :: getContact ( ) const {
	return getFullOrCompact ( "Contact", 'm' );
}

void MIMEInfo :: setContact ( const ss :: string & v ) {
	set ( getFormString("Contact"), v );
}

void MIMEInfo :: setContact ( const URL & url ) {
	setContact ( url.str ( ) );
}

ss :: string MIMEInfo :: getSubject ( ) const {
	return getFullOrCompact ( "Subject", 's' );
}

void MIMEInfo :: setSubject ( const ss :: string & v ) {
	set ( getFormString("Subject"), v );
}

ss :: string MIMEInfo :: getTo ( ) const {
	return getFullOrCompact ( "To", 't' );
}

void MIMEInfo :: setTo ( const ss :: string & v ) {
	set ( getFormString("To"), v );
}

ss :: string MIMEInfo :: getVia ( ) const {
	return getFullOrCompact ( "Via", 'v' );
}

void MIMEInfo :: setVia ( const ss :: string & v ) {
	set ( getFormString("Via"), v );
}

void MIMEInfo :: setContentLength ( unsigned v ) {
	ss :: ostringstream os;
	os << v;
	set ( getFormString("Content-Length"), os.str ( ) );
}

ss :: string MIMEInfo :: getCSeq ( ) const {
	return get ( "CSeq" );
}

void MIMEInfo :: setCSeq ( const ss :: string & v ) {
	set ( "CSeq", v );
}

ss :: string MIMEInfo :: getAccept ( ) const {
	return get ( "Accept" );
}

void MIMEInfo :: setAccept ( const ss :: string & v ) {
	set ( "Accept", v );
}

ss :: string MIMEInfo :: getAcceptEncoding ( ) const {
	return get ( "Accept-Encoding" );
}

void MIMEInfo :: setAcceptEncoding ( const ss :: string & v ) {
	set ( "Accept-Encoding", v );
}

ss :: string MIMEInfo :: getAcceptLanguage ( ) const {
	return get ( "Accept-Language" );
}

void MIMEInfo :: setAcceptLanguage ( const ss :: string & v ) {
	set ( "Accept-Language", v );
}

ss :: string MIMEInfo :: getSupported ( ) const {
	return get ( "Supported" );
}

void MIMEInfo :: setSupported ( const ss :: string & v ) {
	set ( "Supported", v );
}

ss :: string MIMEInfo :: getAllow ( ) const {
	return get ( "Allow" );
}

void MIMEInfo :: setAllow ( const ss :: string & v ) {
	set ( "Allow", v );
}

ss :: string MIMEInfo :: getUserAgent ( ) const {
	return get ( "User-Agent" );
}

void MIMEInfo :: setUserAgent ( const ss :: string & v ) {
	set ( "User-Agent", v );
}

StringVector MIMEInfo :: getRoute ( ) const {
	return getRouteList ( "Route" );
}

void MIMEInfo :: setRoute ( const StringVector & v ) {
//		setRouteList ( "Route",  v );
		if(v.size() > 1)
			PSYSTEMLOG(Info, "MIMEInfo :: setRoute: size = " << v.size() << "; [0] = " << v[0] );
		else
			PSYSTEMLOG(Info, "MIMEInfo :: setRoute: size = 0" );

		for(unsigned i = 0; i < v.size(); ++i)
		{
			strRoute_ += v[i];
            PSYSTEMLOG(Info, "MIMEInfo :: setRoute: " << v[ i ]<< "; #" << i );
			if(i < v.size() - 1)
			{
				strRoute_ += ", ";
			}
		}
}

void MIMEInfo :: printRecordRoute ( std :: ostream & os ) const
{
	if(false == getRecordRouteRequired())
		return;

	if(false == vectorRoute_.empty())
	{
		os << "Record-Route:";
	}

	for(unsigned i = 0; i < vectorRoute_.size(); ++i)
	{
		os << vectorRoute_[i]/* << "\r\n"*/;
		if(int(vectorRoute_.size() - i) > 1)
			os << ", ";
	}

	if(false == vectorRoute_.empty())
	{
		os << "\r\n";
	}

	if(false == strRoute_.empty())
	{
		//os << "Route: <sip:" << strRoute_ << ";lr>\r\n";
		os << "Route: " << strRoute_ << "\r\n";
	}
}

StringVector MIMEInfo :: getRecordRoute ( ) const {
	return vectorRoute_;// getRouteList ( "Record-Route" );
}

void MIMEInfo :: setRecordRoute ( const StringVector & v ) {
	if ( ! v.empty ( ) )
	{
		for ( unsigned i = 0; i < v.size ( ); i ++ ) {
			set ( "Record-Route", v [ i ] );
//            PSYSTEMLOG( Info, "MIMEInfo :: setRecordRoute: " << v [ i ] << "; #" << i );
		}
	}
}

void MIMEInfo :: addRecordRoute( const ss :: string & url, bool ins )
{
	/// creates new record
	ss :: ostringstream os;
	if(ins == true)
		os << "<sip:" << url << ";lr>";
	else
		os << url;

	StringVector::const_iterator cit = std :: find(vectorRoute_.begin(), vectorRoute_.end(), os.str ( ));
	if(cit == vectorRoute_.end())
		vectorRoute_.insert ( vectorRoute_.begin(), os.str ( ) );

//    PSYSTEMLOG( Info, "MIMEInfo :: addRecordRoute: " << os.str() );

}

void MIMEInfo :: clearRecordRoute()
{
	vectorRoute_.clear();
}

void MIMEInfo :: setRoute(const ss :: string& url, bool ins)
{
	if( url.size() <= 0 )
		return;

	ss :: ostringstream os;
	if(ins == true)
		os << "<sip:" << url << ";lr>";
	else
		os << url;

	strRoute_ = os.str();
//	PSYSTEMLOG(Info, "MIMEInfo :: setRoute: " << strRoute_ );
}

bool MIMEInfo :: getRecordRouteRequired() const
{
	return isRecordRouteRequired_;
}
void MIMEInfo :: setRecordRouteRequired(bool isRecordRouteRequired)
{
	isRecordRouteRequired_ = isRecordRouteRequired;
}


StringVector MIMEInfo :: getRouteList ( const ss :: string & name ) const {
	StringVector routeSet;
	ss :: string s = get ( name );
	ss :: string :: size_type left;
	ss :: string :: size_type right = 0;
	while ( ( left = s.find ( '<', right ) ) != ss :: string :: npos &&
		( right = s.find ( '>', left ) ) != ss :: string :: npos &&
		( right - left ) > 5 )
		routeSet.push_back ( s.substr ( left + 1, right - left - 2 ) );
	return routeSet;
}

void MIMEInfo :: setRouteList ( const ss :: string & name, const StringVector & v ) {
	ss :: ostringstream os;
	for ( unsigned i = 0; i < v.size ( ); i ++ ) {
		if ( i > 0 )
			os << ',';
		os << '<' << v [ i ] << '>';
	}
	set ( name, os.str ( ) );
}

ss :: string MIMEInfo :: get ( const ss :: string & name ) const {
	typedef values_t :: index < attribute > :: type AttributeByName;
	const AttributeByName & attributeByName = values.get < attribute > ( );
	AttributeByName :: iterator i = attributeByName.find ( name );
	if ( i == attributeByName.end ( ) )
    {
        return "";
    }
	return i -> second;
}

void MIMEInfo :: set ( const ss :: string & name, const ss :: string & val ) {
	typedef values_t :: index < attribute > :: type AttributeByName;
	AttributeByName & attributeByName = values.get < attribute > ( );
	AttributeByName :: iterator i = attributeByName.find ( name );
	if ( i == attributeByName.end ( ) )
	{
        values.push_back ( values_t :: value_type ( name, val ) );
    }
	else
    {
        attributeByName.replace ( i, values_t :: value_type ( name, val ) );
    }
}

ss :: string MIMEInfo :: getFullOrCompact ( const ss :: string & fullForm, char compactForm ) const {
    if ( values.get < attribute > ( ).count ( fullForm ) )
	{
		return get ( fullForm );
	}
	ss :: string t;
	t = compactForm;
	return get ( t );
}

ss :: string MIMEInfo :: getFormString(const ss :: string& name) const
{
	ss :: string strName;
	switch(getFieldForm(name))
	{
	case ffAbsentIntoMime:
		strName = name;
		break;
	case ffCompactForm:
	{
		MapCompactToLongForm::const_iterator mci = longFormMap_.find(name);
		if(mci == longFormMap_.end())
		{
			strName = name;
		}
		else
		{
			strName = mci->second;
		}
		break;
	}
	case ffLongForm:
		strName = name;
		break;
	case ffUnknownForm:
		strName = name;
		break;
	}

	return strName;
}

MIMEInfo :: FieldForm MIMEInfo :: getFieldForm(const ss :: string & fieldName) const
{
	if(true == fieldName.empty())
	{
		return ffAbsentIntoMime;
	}

	typedef values_t :: index < attribute > :: type AttributeByName;
	const AttributeByName & attributeByName = values.get < attribute > ( );
	const AttributeByName :: iterator i = attributeByName.find(fieldName);
	if( i != attributeByName.end() )
	{
		if(longFormMap_.find(fieldName) != longFormMap_.end())
		{
			return ffLongForm;
		}
		else
		{
			if(compactFormMap_.find(fieldName) != compactFormMap_.end())
			{
				return ffCompactForm;
			}
			else
			{
				return ffUnknownForm;
			}
		}
	}
	else
	{ // Not present in this form
		FieldForm ff;
		ss :: string strNewForm;
		MapCompactToLongForm::const_iterator cit;
		cit = longFormMap_.find(fieldName);
		if(cit != longFormMap_.end())
		{
			ff = ffCompactForm;
			strNewForm = cit->second;
		}
		else
		{
			cit = compactFormMap_.find(fieldName);
			if(cit != compactFormMap_.end())
			{
				ff = ffLongForm;
				strNewForm = cit->second;
			}
			else
			{
				return ffAbsentIntoMime;
			}
		}

		AttributeByName :: iterator it = attributeByName.find(strNewForm);
		if( it != attributeByName.end() )
		{
			return ff;
		}
	}
	return ffAbsentIntoMime;
}
/*
static ss :: string parseToTag(const ss :: string& strFrom)
{
	ss :: string :: size_type stBeginTag = strFrom.find("tag=");
	ss :: string strTag;
	if( ss :: string :: npos != stBeginTag )
	{
		strTag = strFrom.substr(stBeginTag + strlen("tag=") );
	}
	return strTag;
}
*/
bool MIMEInfo :: addMIME ( const ss :: string & line ) {
	ss :: string :: size_type colonPos = line.find ( ':' );
	if ( colonPos == ss :: string :: npos ) {
		PTRACE ( 1, "no colon in mime string " << line );
		return false;
	}

//	PSYSTEMLOG(Info, "MIMEInfo :: addMIME line = " << line);

	ss :: string fieldName  = trim ( line.substr ( 0, colonPos ) );
	ss :: string fieldValue = trim ( line.substr ( colonPos + 1 ) );

	if ( values.get < attribute > ( ).count ( fieldName ) )
		fieldValue = get ( fieldName ) + '\n' + fieldValue;

	if(fieldName == "Record-Route")
	{
		ss :: string :: size_type left( ss :: string :: npos );
		ss :: string :: size_type right( ss :: string :: npos );
//		PSYSTEMLOG(Info, "parse Record-Route value: " << fieldValue);

		const char *str = fieldValue.data ( );
		ss :: string :: size_type sz = fieldValue.size ( );
		for( ss :: string :: size_type i = 0; i < sz; ++i)
		{
			if(str[i] == ':')
			{
			    if(left == ss :: string :: npos)
				left = i;
			}
			else if(str[i] == ';' && left != ss :: string :: npos)
			{
				right = i;
				ss :: string str1 = fieldValue.substr ( left + 1, right - left - 1 );
//				PSYSTEMLOG(Info, "parse Record-Route: " << str1);
				addRecordRoute( str1 );
				left = ss :: string :: npos;
			}
		}

		if(right == ss :: string :: npos && false == fieldValue.empty())
		{ // If '<sip:' and ';lr>' is absent in the fieldValue
			addRecordRoute(fieldValue);
		}
	}
	else if(fieldName == "Route")
	{
		; // Do nothing.
	}
        else if(fieldName == "Remote-Party-ID" )
        {
            setRemotePartyId (fieldValue);
//    PSYSTEMLOG(Info, "!!!!!!!!!!!!!!!!!: " << fieldName << ", fieldValue = " << fieldValue );
        }
        else if( fieldName == "Privacy" )
        {
            setPrivacy (fieldValue);
            PSYSTEMLOG(Info, "!!!!!!!!!!!!!!!!!: " << fieldName << ", fieldValue = " << fieldValue );
        }
    	else
	{
		set ( fieldName, fieldValue );
		if(fieldName == "From" || fieldName == "from" || fieldName == "f" )
		{
			strRealFrom_ = getFrom();
		}
		if(fieldName == "Contact" || fieldName == "m" )
		{
			strContact_ = fieldValue;
//            PSYSTEMLOG(Info, "MIMEInfo :: addMIME: Contact = " << fieldValue);
		}/*
		else if(fieldName == "Privacy" && fieldValue == "id" )
		{
            // PSYSTEMLOG(Info, "MIMEInfo :: addMIME: Anonymous. GetGrom " << getFrom() << "; Tag = " << parseToTag(getFrom()) );
            ss :: string fromTag = "\"Anonymous\" <sip:Anonymous@Anonymous.invalid>";
            fromTag += parseToTag(getFrom());
          //  setFrom(fromTag);
            PSYSTEMLOG(Info, "MIMEInfo :: addMIME: Anonymous. GetGrom " << getFrom() << "; fromTag = " << fromTag);//"; Tag = " << parseToTag(getFrom())  );
		}
        */
	}

	return true;
}

ss :: string MIMEInfo :: getContactString() const
{
	return strContact_;
}

ss :: string MIMEInfo :: getRealFrom() const
{
	return strRealFrom_;
}

ss :: string MIMEInfo :: getRemotePartyID() const
{
    return get ( "remote-party-id" );
//    return m_remotePartyID;
}

void MIMEInfo :: setRemotePartyID( const ss :: string& remotePartyID )
{
    set ( "remote-party-id", remotePartyID );
//    m_remotePartyID = remotePartyID;
}


void MIMEInfo :: addPrivacyHeader()
{
	isPrivecyHeader_ = true;
}

void MIMEInfo :: setRemotePartyId( const ss :: string& url )
{
    m_remotePartyID = url;
    if(false == url.empty())
    {
        typedef values_t :: index < attribute > :: type AttributeByName;
        AttributeByName & attributeByName = values.get < attribute > ( );
        AttributeByName :: iterator i = attributeByName.find ( "Remote-Party-ID" );
        if( i == attributeByName.end() )
        {
            set ( "Remote-Party-ID", url );
            PSYSTEMLOG(Info, "MIMEInfo :: setRemotePartyId: " << url );
        }
    }
}
											
ss :: string MIMEInfo :: getRemotePartyId( ) const
{
    ss :: string strRet;
    
    if(m_remotePartyID.empty())
        strRet = get ( "Remote-Party-ID" );
    else
        strRet = m_remotePartyID;

    PSYSTEMLOG(Info, "MIMEInfo :: getRemotePartyId: " << strRet);
    return strRet;
}
void MIMEInfo :: setPrivacy( const ss :: string& url )
{
    if(false == url.empty())
    {
        typedef values_t :: index < attribute > :: type AttributeByName;
        AttributeByName & attributeByName = values.get < attribute > ( );
        AttributeByName :: iterator i = attributeByName.find ( "Privacy" );
        if( i == attributeByName.end() )
        {
            set ( "Privacy", url );
            PSYSTEMLOG(Info, "MIMEInfo :: setPrivacy: " << url );
        }
    }
}
										    
ss :: string MIMEInfo :: getPrivacy() const
{
    ss :: string strRet;
    
    strRet = get ( "Privacy" );
	
    PSYSTEMLOG(Info, "MIMEInfo :: getPrivacy: " << strRet);
    return strRet;
}
		
												    
										    				    											
ss :: string MIMEInfo :: getWWWAuthenticate ( ) const {
	return get ( "WWW-Authenticate" );
}

void MIMEInfo :: setWWWAuthenticate ( const ss :: string & v ) {
	set ( "WWW-Authenticate", v );
}

ss :: string MIMEInfo :: getAuthorization ( ) const {
	return get ( "Authorization" );
}

void MIMEInfo :: setAuthorization ( const ss :: string & v ) {
	set ( "Authorization", v );
}

ss :: string MIMEInfo :: getProxyAuthorization ( ) const {
	return get ( "Proxy-Authorization" );
}

void MIMEInfo :: setProxyAuthorization ( const ss :: string & v ) {
	set ( "Proxy-Authorization", v );
}

ss :: string MIMEInfo :: getProxyAuthenticate ( ) const {
	return get ( "Proxy-Authenticate" );
}

void MIMEInfo :: setProxyAuthenticate ( const ss :: string & v ) {
	set ( "Proxy-Authenticate", v );
}

ss :: string MIMEInfo :: getExpires ( ) const {
	return get ( "Expires" );
}

void MIMEInfo :: setExpires ( const ss :: string & v ) {
	set ( "Expires", v );
}

void MIMEInfo :: setPAssertID( const ss :: string & newAssertId )
{
//	PSYSTEMLOG(Info, "setPAssertID: newAssertId=" << newAssertId);
	strPAsserId_ = newAssertId;
	if(false == newAssertId.empty())
	{
		typedef values_t :: index < attribute > :: type AttributeByName;
		AttributeByName & attributeByName = values.get < attribute > ( );
		AttributeByName :: iterator i = attributeByName.find ( "P-Asserted-Identity" );
		if( i == attributeByName.end() )
		{
			set ( "P-Asserted-Identity", newAssertId );
		}

	}
}

void MIMEInfo :: removePAssertID( )
{	typedef values_t :: index < attribute > :: type AttributeByName;
	AttributeByName & attributeByName = values.get < attribute > ( );
	AttributeByName :: iterator i = attributeByName.find ( "P-Asserted-Identity" );
	if ( i != attributeByName.end ( ) )
		attributeByName.erase( i );

	strPAsserId_.clear();
}

ss :: string MIMEInfo :: getPAssertID() const
{
	return strPAsserId_;
}

enum {
	SIP_VER_MINOR = 0,
	SIP_VER_MAJOR = 2
};

PDU :: PDU ( ) : method ( mNum ), statusCode ( scIllegal ), versionMajor ( SIP_VER_MAJOR ),
	versionMinor ( SIP_VER_MINOR ), mime(false), sdp ( 0 ), m_Content ( 0 ) { }

PDU :: PDU ( const PDU & p ) : method ( p.method ), statusCode ( p.statusCode ), uri_ ( p.uri_ ),
	versionMajor ( p.versionMajor ), versionMinor ( p.versionMinor ), info ( p.info ),
	mime ( p.mime ), entityBody ( p.entityBody ), sdp ( p.sdp ? new SessionDescription ( * p.sdp ) : 0 ),
	m_Content ( 0 ) {
	if ( p.m_Content ) {
		try {
			m_Content = new Content ( * p.m_Content );
		} catch ( ... ) {
			delete sdp;
			throw;
		}
	}
}

PDU :: PDU ( Methods m, const URL & u ) : method ( m ), statusCode ( scIllegal ), uri_ ( u ),
	versionMajor ( SIP_VER_MAJOR ), versionMinor ( SIP_VER_MINOR ), mime(false), sdp ( 0 ), m_Content(0)
{
}

PDU & PDU :: operator= ( const PDU & p ) {
	if ( this == & p )
		return * this;
	method = p.method;
	statusCode = p.statusCode;
	uri_ = p.uri_;
	versionMajor = p.versionMajor;
	versionMinor = p.versionMinor;
	info = p.info;
	mime = p.mime;
	entityBody = p.entityBody;
	safeDel ( sdp );
	if ( p.sdp )
		sdp = new SessionDescription ( * p.sdp );
	safeDel ( m_Content );
	if ( p.m_Content )
		m_Content = new Content ( * p.m_Content );
	return * this;
}

PDU :: ~PDU ( ) {
	safeDel ( sdp );
	safeDel ( m_Content );
}

ss :: string PDU :: getTransactionID ( ) const {
	ss :: ostringstream os;
	os << mime.getFrom ( ) << mime.getCSeqIndex ( );
	return os.str ( );
}

PDU :: Methods PDU :: getMethod ( ) const {
	return method;
}

void PDU :: setMethod ( Methods m ) {
	method = m;
}

int PDU :: getStatusCode ( ) const {
	return statusCode;
}

void PDU :: setStatusCode ( int sc ) {
	statusCode = sc;
}

const URL & PDU :: getURI ( ) const {
	return uri_;
}

void PDU :: setURI ( const URL & u ) {
	uri_ = u.shortForm ( );
}

unsigned PDU :: getVersionMajor ( ) const {
	return versionMajor;
}

unsigned PDU :: getVersionMinor ( ) const {
	return versionMinor;
}

const ss :: string & PDU :: getEntityBody ( ) const {
	return entityBody;
}

void PDU :: setEntityBody ( const ss :: string & e ) {
	entityBody = e;
}

const ss :: string & PDU :: getInfo ( ) const {
	return info;
}

void PDU :: setInfo ( const ss :: string & i ) {
	info = i;
}

const MIMEInfo & PDU :: getMIME ( ) const {
	return mime;
}

MIMEInfo & PDU :: getMIME ( ) {
	return mime;
}

void PDU :: setMIME ( const MIMEInfo & m ) {
	mime = m;
}

bool PDU :: hasSDP ( ) const {
	return bool(sdp);
}

SessionDescription & PDU :: getSDP ( ) const {
	return * sdp;
}

void PDU :: setSDP ( SessionDescription * s ) {
	safeDel ( sdp );
	sdp = s;
}

void PDU :: setSDP ( const SessionDescription & s ) {
	safeDel ( sdp );
	sdp = new SessionDescription ( s );
}

const Content * PDU :: getContent ( ) const {
	return m_Content;
}

static const char * const methodNames [ PDU :: mNum ] = {
	"INVITE",
	"ACK",
	"OPTIONS",
	"BYE",
	"CANCEL",
	"REGISTER",
	"REFER",
	"MESSAGE",
	"NOTIFY",
	"SUBSCRIBE",
	"INFO"/*,
	"UnAuthorized"*/
};

bool PDU :: readFrom ( std :: istream & is ) {
	ss :: string cmd;
	if ( ! getline ( is, cmd ) ) {
		PSYSTEMLOG ( Error, "Read SIP cmd failed: " << cmd );
		return false;
	}
	cmd = trim ( cmd );
	if ( cmd.empty ( ) ) {
		PSYSTEMLOG ( Error, "No Request-Line or Status-Line received" );
		return false;
	}
	if ( ! mime.readFrom ( is ) ) {
		PSYSTEMLOG ( Error, "Read SIP MIME failed: " << cmd << mime );
		return false;
	}
	if ( ! cmd.compare ( 0, 4, "SIP/" ) ) {
		ss :: string :: size_type space = cmd.find ( ' ' );
		if ( space == ss :: string :: npos ) {
			PSYSTEMLOG ( Error, "Bad Status-Line received: " << cmd );
			return false;
		}
		ss :: string :: size_type dot = cmd.find ( '.' );
		if ( dot == ss :: string :: npos ) {
			PSYSTEMLOG ( Error, "Bad Status-Line received: " << cmd );
			return false;
		}
		versionMajor = atoi ( cmd.substr ( 4 ) );
		versionMinor = atoi ( cmd.substr ( dot + 1 ) );
		statusCode = StatusCodes ( atoi ( cmd.substr ( ++ space ) ) );
		space = cmd.find ( ' ', space );
		if ( space == ss :: string :: npos ) {
			PSYSTEMLOG ( Error, "Bad Status-Line received: " << cmd );
			return false;
		}
		info = trim ( cmd.substr ( space ) );
		uri_ = URL ( );
	} else {
		// parse the method, URI and version
		ss :: istringstream is2 ( cmd );
		ss :: string cmds0, cmds1, cmds2;
		is2 >> cmds0 >> cmds1 >> cmds2;
		ss :: string :: size_type dot = cmds2.find ( '.' );
		if ( dot == ss :: string :: npos || cmds2.compare ( 0, 4, "SIP/" ) ) {
			PSYSTEMLOG ( Error, "Bad Request-Line received: " << cmd );
			return false;
		}
		int i = 0;
		while ( i < mNum && cmds0 != methodNames [ i ] )
			i ++;
		method = Methods ( i );

		uri_ = cmds1;
		versionMajor = atoi ( cmds2.substr ( 4 ) );
		versionMinor = atoi ( cmds2.substr ( dot + 1 ) );
		info = "";
	}
	if ( versionMajor < 2 ) {
		PSYSTEMLOG ( Error, "Invalid SIP version received: " << versionMajor << '.' << versionMinor );
		return false;
	}

	unsigned contentLength = mime.getContentLength ( );
	if ( ( int ( contentLength ) < 0 ) || ( int ( contentLength ) > PDU :: maxSize ) ) {
		PSYSTEMLOG ( Error, "Invalid ContentLength " << ( int ) ( contentLength ) );
		return false;
	}
	entityBody.resize ( contentLength );
	safeDel ( sdp );
	if ( ! contentLength )
	{
//        PSYSTEMLOG ( Info, "PDU :: readFrom. contentLength = 0" );
        return true;
    }
	is.read ( & entityBody [ 0 ], contentLength );
	if ( compareNoCase ( mime.getContentType ( ), "application/sdp" ) == 0 ) {
		sdp = new SessionDescription ( );
		if ( ! sdp -> decode ( entityBody ) ) {
			PSYSTEMLOG ( Error, "cannot decode SDP" );
			safeDel ( sdp );
			return false;
		}
	} else {
		m_Content = new Content ( contentLength );
		if ( ! m_Content -> decode ( entityBody ) )
			PSYSTEMLOG ( Error, "Can not decode Content." );
		PSYSTEMLOG ( Info, "PDU :: readFrom: ContentData = " << m_Content -> getContentData ( ) <<
			"; size = " << m_Content -> getContentLength ( ) );
	}

	return true;
}

bool PDU :: read ( IxcUDPSocket & sock ) {
	const int bufferSize = 4096;
	char buffer [ bufferSize ];
	if ( ! sock.Read ( buffer, bufferSize ) ) {
		PSYSTEMLOG ( Error, "can't read SIP packet: " << sock.GetErrorText ( PChannel :: LastReadError ) );
		return false;
	}
    std :: size_t sz = sock.GetLastReadCount ( );
	ss :: string buf ( buffer, sz );
	if ( buf == "\r\n" )
	{
        return false;
    }
//	PIPSocket::Address  PeerAddr;
//	WORD PeerPort;
//	sock.GetLastReceiveAddress ( PeerAddr, PeerPort );
//	PSYSTEMLOG ( Info, "received SIP packet from " << PeerAddr.AsString ( ) << ":" << PeerPort << " (sz=" << sz << "):\n" << buf );
	ss :: istringstream is ( buf );
	if ( readFrom ( is ) )
	{
        return true;
    }
	return false;
}

void PDU :: setContact ( const ss :: string & strContact ) {
/*	strContact_ = strContact;
	if ( false == strContact.empty ( ) ) {
		unsigned left = strContact_.find ( '<' );
		if(left == ss :: string :: npos)
		{
			ss :: string strURI = strContact_;
			int right = strURI.rfind ( ':' );
			uri_ = strURI.substr ( 0, right );
		}
		else
		{
			ss :: string strURI = strContact_.substr ( left + 1 );
			int right = strURI.rfind ( ':' );
			uri_ = strURI.substr ( 0, right );
		}
	}
*/
	strContact_ = strContact;
	if ( false == strContact.empty ( ) )
	{
		ss :: string :: size_type leftSip = strContact.find ( "sip:" );
		ss :: string :: size_type right = string :: npos;

		if(leftSip == string :: npos)
		{
			uri_ = ss :: string("");
		}
		ss :: string terminators(":;>");
		ss :: string strURI = strContact.substr(leftSip + std :: strlen("sip:"), ss :: string :: npos);

		ss :: string str1 = strURI;
		do
		{
			str1 = str1.substr ( 0, right );
			right = str1.find_last_of ( terminators );
		} while (right != ss :: string :: npos);

		uri_ = str1;
	}
}

void PDU :: printOn ( std :: ostream & os ) {
	if ( sdp ) {
		entityBody = sdp -> str ( );
		mime.setContentType ( "application/sdp" );
	} else if ( m_Content )
		entityBody = m_Content->getContentData();
	else
		entityBody.clear ( );
	mime.setContentLength ( int ( entityBody.size ( ) ) );

	ss :: string strURI = uri_.shortForm ( );

	if ( method != mNum )
		os << methodNames [ method ] << ' ' << strURI << ' ';
	os << "SIP/" << versionMajor << '.' << versionMinor;

	if ( method == mNum )
		os << ' ' << unsigned ( statusCode ) << ' ' << info;

	os << "\r\n" << std :: setfill ( '\r' ) << mime << std :: setfill ( ' ' ) << entityBody;
}

bool PDU :: write ( PChannel & transport ) {
	ss :: ostringstream os;
	printOn ( os );
#if PTRACING
	if ( PTrace :: CanTrace ( 4 ) )
		PTRACE ( 4, "Sending SIP PDU on " << transport << '\n' << os.str ( ) );
	else if ( ! method )
		PTRACE ( 3, "Sending SIP PDU: " << methodNames [ method ] << ' ' << uri_ << " on " <<
			transport << " " << os.str());
	else
		PTRACE ( 3, "Sending SIP PDU: " << unsigned ( statusCode ) << ' ' << info << " on " <<
			transport << "; " << os.str());
#endif

	if ( transport.WriteString ( os.str ( ).c_str ( ) ) )
	{
        return true;
    }
	PTRACE ( 1, "Write SIP PDU failed: " << transport.GetErrorText ( PChannel :: LastWriteError ) );
	return false;
}
////////////////////////////////////////////////////////////////////////////////
Content :: Content (int len) : m_ContentLength(len) { }

bool Content :: decode ( const ss :: string & str ) {
        setContentData ( str );
	return true;
}

const ss :: string & Content :: getContentData ( ) const {
	return m_ContentData;
}

void Content :: setContentData ( const ss :: string & d ) {
	m_ContentData = d;
}

int Content :: getContentLength ( ) const {
	return m_ContentLength;
}

void Content :: setContentLength ( int l ) {
	m_ContentLength = l;
}

const ss :: string & Content :: getContentType ( ) const {
	return m_ContentType;
}

void Content :: getContentType ( const ss :: string & t ) {
	m_ContentType = t;
}

//bool ciCharLess(char c1, char c2)
//{
//	return tolower(static_cast<unsigned char>(c1)) < tolower(static_cast<unsigned char>(c2));
//}

bool ciCharLess(char c1, char c2)
{
	return std::tolower(static_cast<unsigned char>(c1)) < std :: tolower(static_cast<unsigned char>(c2));
}

bool CiStringCompare :: operator()(const ss :: string& lhs, const ss :: string& rhs) const {
//	bool ret = !!compareNoCase(lhs, rhs);
//	PSYSTEMLOG(Info, "CiStringCompare :: operator(): 1st " << lhs << " 2nd " << rhs << " res " << ret);
//	return ret;
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), ciCharLess);
};

} // namespace SIP


