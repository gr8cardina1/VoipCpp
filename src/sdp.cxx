#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include "sdp.hpp"
#include <boost/algorithm/string/trim.hpp>
#include <stdexcept>
#include <boost/spirit/include/classic_parse_tree.hpp>
#include "rightparser.hpp"
#include "sdpgrammars.hpp"
#include <tr1/functional>
#include <ptlib.h>
#include "ntptimestamp.hpp"

namespace SDP {

struct attr { };
typedef std :: pair < ss :: string, ss :: string > AttrType;
typedef boost :: multi_index :: multi_index_container < AttrType,
	boost :: multi_index :: indexed_by < boost :: multi_index :: sequenced < >,
	boost :: multi_index :: ordered_non_unique < boost :: multi_index :: tag < attr >,
	boost :: multi_index :: member < AttrType, ss :: string, & AttrType :: first > > >,
	__SS_ALLOCATOR < AttrType > > AttributeFields;


struct ConnectionField {
	ss :: string connectionAddress;
};

const ss :: string mediaAudio ( "audio" );
const ss :: string mediaImage ( "image" );
const ss :: string protoRtpAvp ( "RTP/AVP" );
const ss :: string protoUdptl ( "udptl" );

const ss :: string PCMU ( "0" );	// G.711 u-Law
const ss :: string FS1016 ( "1" );	// Federal Standard 1016 CELP
const ss :: string G721 ( "2" );	// ADPCM - Subsumed by G.726
const ss :: string G726 = G721;
const ss :: string GSM ( "3" );		// GSM 06.10
const ss :: string G7231 ( "4" );	// G.723.1 at 6.3kbps or 5.3 kbps
const ss :: string DVI4_8k ( "5" );	// DVI4 at 8kHz sample rate
const ss :: string DVI4_16k ( "6" );	// DVI4 at 16kHz sample rate
const ss :: string LPC ( "7" );		// LPC-10 Linear Predictive CELP
const ss :: string PCMA ( "8" );	// G.711 A-Law
const ss :: string G722 ( "9" );	// G.722
const ss :: string L16_Stereo ( "10" );	// 16 bit linear PCM
const ss :: string L16_Mono ( "11" );	// 16 bit linear PCM
const ss :: string G723 ( "12" );	// G.723
const ss :: string CN ( "13" );		// Confort Noise
const ss :: string MPA ( "14" );	// MPEG1 or MPEG2 audio
const ss :: string G728 ( "15" );	// G.728 16kbps CELP
const ss :: string DVI4_11k ( "16" );	// DVI4 at 11kHz sample rate
const ss :: string DVI4_22k ( "17" );	// DVI4 at 22kHz sample rate
const ss :: string G729 ( "18" );	// G.729 8kbps
const ss :: string Cisco_CN ( "19" );	// Cisco systems comfort noise (unofficial)
const ss :: string CelB ( "25" );	// Sun Systems Cell-B video
const ss :: string JPEG ( "26" );	// Motion JPEG
const ss :: string H261 ( "31" );	// H.261
const ss :: string MPV ( "32" );	// MPEG1 or MPEG2 video
const ss :: string MP2T ( "33" );	// MPEG2 transport system
const ss :: string H263 ( "34" );	// H.263
const ss :: string T38 ( "t38" );
const ss :: string TelephoneEvent ( "96" );

class MediaDescription :: Impl : public Allocatable < __SS_ALLOCATOR > {
	struct MediaField {
		ss :: string media;
		unsigned port;
		ss :: string proto;
		MediaFormatVector fmts;
	} mediaField;
	ss :: string informationField;
	std :: vector < ConnectionField, __SS_ALLOCATOR < ConnectionField > > connectionFields;
	StringVector bandwidthFields;
	ss :: string keyField;
	AttributeFields attributeFields;
	void cacheAttribute ( const ss :: string & value, void ( MediaFormat :: * f ) ( const ss :: string & s ) );
public:
	Impl ( const ss :: string & media, unsigned port, const ss :: string & proto );
	void printOn ( std :: ostream & os ) const;
	void addFmt ( const ss :: string & s );
	void setInformationField ( const ss :: string & s );
	void addConnectionField ( const ss :: string & ip );
	void clearConnectionField ( );
	void addBandwidthField ( const ss :: string & s );
	void setKeyField ( const ss :: string & s );
	void addAttributeField ( const ss :: string & key, const ss :: string & value );
	void delAttributeField ( const char * key );
	std :: size_t getFmtsSize ( ) const;
	const ss :: string & getMedia ( ) const;
	const ss :: string * getAttributeValue ( const char * key ) const;
	void getAttributeValues ( const char * key, StringVector & v ) const;
	const MediaFormatVector & getFmts ( ) const;
	const ss :: string * getConnectionAddress ( ) const;
	unsigned getPort ( ) const;
};

MediaDescription :: Impl :: Impl ( const ss :: string & media, unsigned port, const ss :: string & proto ) {
	mediaField.media = media;
	mediaField.port = port;
	mediaField.proto = proto;
}

void MediaDescription :: Impl :: printOn ( std :: ostream & os ) const {
	os << "m=" << mediaField.media << ' ' << mediaField.port << ' ' << mediaField.proto;
	if ( mediaField.fmts.empty ( ) )
		throw std :: runtime_error ( "empty mediaField.fmts" );
	for ( MediaFormatVector :: const_iterator i = mediaField.fmts.begin ( ); i != mediaField.fmts.end ( ); ++ i )
		os << ' ' << i -> getFmt ( );
	os << "\r\n";
	if ( ! informationField.empty ( ) )
		os << "i=" << informationField << "\r\n";
	for ( unsigned i = 0; i < connectionFields.size ( ); i ++ )
		os << "c=IN IP4 " << connectionFields [ i ].connectionAddress << "\r\n";
	for ( unsigned i = 0; i < bandwidthFields.size ( ); i ++ )
		os << "b=" << bandwidthFields [ i ] << "\r\n";
	if ( ! keyField.empty ( ) )
		os << "k=" << keyField << "\r\n";
	for ( AttributeFields :: const_iterator i = attributeFields.begin ( ); i != attributeFields.end ( ); ++ i ) {
		os << "a=" << i -> first;
		if ( ! i -> second.empty ( ) )
			os << ':' << i -> second;
		os << "\r\n";
	}
}

void MediaDescription :: Impl :: addFmt ( const ss :: string & s ) {
	if ( ! mediaField.fmts.push_back ( MediaFormat ( s ) ).second )
		throw std :: runtime_error ( std :: string ( "duplicate fmt " ) + s.c_str ( ) );
}

void MediaDescription :: Impl :: setInformationField ( const ss :: string & s ) {
	informationField = s;
}

void MediaDescription :: Impl :: addConnectionField ( const ss :: string & ip ) {
	ConnectionField f;
	f.connectionAddress = ip;
	connectionFields.push_back ( f );
}

void MediaDescription :: Impl :: clearConnectionField ( ) {
	connectionFields.clear ( );
}

void MediaDescription :: Impl :: addBandwidthField ( const ss :: string & s ) {
	bandwidthFields.push_back ( s );
}

void MediaDescription :: Impl :: setKeyField ( const ss :: string & s ) {
	keyField = s;
}

void MediaDescription :: Impl :: cacheAttribute ( const ss :: string & value,
	void ( MediaFormat :: * f ) ( const ss :: string & s ) ) {
	ss :: string :: size_type pos = value.find ( ' ' );
	if ( pos == ss :: string :: npos )
		return;
	typedef MediaFormatVector :: index < fmt > :: type ByFmt;
	ByFmt & byFmt = mediaField.fmts.get < fmt > ( );
	ByFmt :: const_iterator i = byFmt.find ( value.substr ( 0, pos ) );
	if ( i == byFmt.end ( ) )
		return;
	byFmt.modify ( i, std :: tr1 :: bind ( f, std :: tr1 :: placeholders :: _1, value.substr ( pos + 1 ) ) );
}

void MediaDescription :: Impl :: addAttributeField ( const ss :: string & key, const ss :: string & value ) {
	AttrType a ( key, value );
	attributeFields.push_back ( a );
	if ( key == "rtpmap" ) {
		cacheAttribute ( a.second, & MediaFormat :: setRtpmap );
		return;
	}
	if ( key == "fmtp" )
		cacheAttribute ( a.second, & MediaFormat :: setFmtp );
}

void MediaDescription :: Impl :: delAttributeField ( const char * key ) {
	typedef AttributeFields :: index < attr > :: type ByAttr;
	ByAttr & byAttr = attributeFields.get < attr > ( );
	byAttr.erase ( key );
}

std :: size_t MediaDescription :: Impl :: getFmtsSize ( ) const {
	return mediaField.fmts.size ( );
}

const ss :: string & MediaDescription :: Impl :: getMedia ( ) const {
	return mediaField.media;
}

const ss :: string * MediaDescription :: Impl :: getAttributeValue ( const char * key ) const {
	typedef AttributeFields :: index < attr > :: type ByAttr;
	const ByAttr & byAttr = attributeFields.get < attr > ( );
	ByAttr :: const_iterator i = byAttr.find ( key );
	if ( i == byAttr.end ( ) )
		return 0;
	return & i -> second;
}

void MediaDescription :: Impl :: getAttributeValues ( const char * key, StringVector & v ) const {
	typedef AttributeFields :: index < attr > :: type ByAttr;
	const ByAttr & byAttr = attributeFields.get < attr > ( );
	std :: pair < ByAttr :: const_iterator, ByAttr :: const_iterator > r = byAttr.equal_range ( key );
	for ( ; r.first != r.second; r.first ++ )
		v.push_back ( r.first -> second );
}

const MediaFormatVector & MediaDescription :: Impl :: getFmts ( ) const {
	return mediaField.fmts;
}

const ss :: string * MediaDescription :: Impl :: getConnectionAddress ( ) const {
	if ( connectionFields.empty ( ) )
		return 0;
	return & connectionFields [ 0 ].connectionAddress;
}

unsigned MediaDescription :: Impl :: getPort ( ) const {
	return mediaField.port;
}

MediaDescription :: MediaDescription ( const ss :: string & media, unsigned port, const ss :: string & proto ) :
	impl ( new Impl ( media, port, proto ) ) { }

MediaDescription :: MediaDescription ( const MediaDescription & m ) : impl ( new Impl ( * m.impl ) ) { }

MediaDescription & MediaDescription :: operator= ( const MediaDescription & m ) {
	MediaDescription ( m ).swap ( * this );
	return * this;
}

void MediaDescription :: swap ( MediaDescription & m ) {
	std :: swap ( impl, m.impl );
}

MediaDescription :: ~MediaDescription ( ) {
	delete impl;
}

void MediaDescription :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

void MediaDescription :: addFmt ( const ss :: string & s ) {
	impl -> addFmt ( s );
}

void MediaDescription :: setInformationField ( const ss :: string & s ) {
	impl -> setInformationField ( s );
}

void MediaDescription :: addConnectionField ( const ss :: string & ip ) {
	impl -> addConnectionField ( ip );
}

void MediaDescription :: clearConnectionField ( ) {
	impl -> clearConnectionField ( );
}

void MediaDescription :: addBandwidthField ( const ss :: string & s ) {
	impl -> addBandwidthField ( s );
}

void MediaDescription :: setKeyField ( const ss :: string & s ) {
	impl -> setKeyField ( s );
}

void MediaDescription :: addAttributeField ( const ss :: string & key, const ss :: string & value ) {
	impl -> addAttributeField ( key, value );
}

void MediaDescription :: delAttributeField ( const char * key ) {
	impl -> delAttributeField ( key );
}

std :: size_t MediaDescription :: getFmtsSize ( ) const {
	return impl -> getFmtsSize ( );
}

const ss :: string & MediaDescription :: getMedia ( ) const {
	return impl -> getMedia ( );
}

const ss :: string * MediaDescription :: getAttributeValue ( const char * key ) const {
	return impl -> getAttributeValue ( key );
}

void MediaDescription :: getAttributeValues ( const char * key, StringVector & v ) const {
	impl -> getAttributeValues ( key, v );
}

const MediaFormatVector & MediaDescription :: getFmts ( ) const {
	return impl -> getFmts ( );
}

const ss :: string * MediaDescription :: getConnectionAddress ( ) const {
	return impl -> getConnectionAddress ( );
}

unsigned MediaDescription :: getPort ( ) const {
	return impl -> getPort ( );
}

unsigned getPtime ( const MediaDescription * media ) {
	if ( const ss :: string * s = media -> getAttributeValue ( "ptime" ) )
		return std :: atoi ( s -> c_str ( ) );
	return 0;
}

class SessionDescription :: Impl : public Allocatable < __SS_ALLOCATOR > {
	struct OriginField {
		ss :: string username;
		unsigned long long sessId;
		unsigned long long sessVersion;
		ss :: string addr;
	} originField;
	ss :: string sessionNameField;
	ss :: string informationField;
	ss :: string uriField;
	StringVector emailFields;
	StringVector phoneFields;
	ConnectionField connectionField;
	StringVector bandwidthFields;
	struct TimeField {
		ss :: string t;
		StringVector r;
	};
	std :: vector < TimeField, __SS_ALLOCATOR < TimeField > > timeFields;
	ss :: string zoneAdjustments;
	ss :: string keyField;
	AttributeFields attributeFields;
	struct media { };
	typedef boost :: multi_index :: multi_index_container < MediaDescription,
		boost :: multi_index :: indexed_by < boost :: multi_index :: sequenced < >,
		boost :: multi_index :: ordered_non_unique < boost :: multi_index :: tag < media >,
		boost :: multi_index :: const_mem_fun < MediaDescription, const ss :: string &,
		& MediaDescription :: getMedia > > >, __SS_ALLOCATOR < MediaDescription > > MediaDescriptionVector;

	MediaDescriptionVector mediaDescriptions;
public:
	explicit Impl ( const ss :: string & s );
	Impl ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie );
	void printOn ( std :: ostream & os ) const;
	const ss :: string & getSessionName ( ) const {
		return sessionNameField;
	}
	void setConnectionAddress ( const ss :: string & caddr );
	void addMediaDescription ( const MediaDescription & m );
	const MediaDescription * getMediaDescription ( const ss :: string & media ) const;
	const ss :: string * getAttributeValue ( const char * key ) const;
	void getAttributeValues ( const char * key, StringVector & v ) const;
	const ss :: string & getConnectionAddress ( ) const;
};

#define SDP_DEFAULT_SESSION_NAME "SS Session"

SessionDescription :: Impl :: Impl ( const ss :: string & oaddr ) {
	originField.username = '-';
	originField.sessId = originField.sessVersion = ntpTimestamp ( );
	originField.addr = oaddr;
	sessionNameField = SDP_DEFAULT_SESSION_NAME;
	TimeField t;
	t.t = "0 0";
	timeFields.push_back ( t );
}

void SessionDescription :: Impl :: printOn ( std :: ostream & os ) const {
	os << "v=0\r\n";
	os << "o=" << originField.username << ' ' << originField.sessId << ' ' << originField.sessVersion << " IN IP4 " <<
		originField.addr << "\r\n";
	os << "s=" << sessionNameField << "\r\n";
	if ( ! informationField.empty ( ) )
		os << "i=" << informationField << "\r\n";
	if ( ! uriField.empty ( ) )
		os << "u=" << uriField << "\r\n";
	for ( unsigned i = 0; i < emailFields.size ( ); i ++ )
		os << "e=" << emailFields [ i ] << "\r\n";
	for ( unsigned i = 0; i < phoneFields.size ( ); i ++ )
		os << "p=" << phoneFields [ i ] << "\r\n";
	if ( ! connectionField.connectionAddress.empty ( ) )
		os << "c=IN IP4 " << connectionField.connectionAddress << "\r\n";
	for ( unsigned i = 0; i < bandwidthFields.size ( ); i ++ )
		os << "b=" << bandwidthFields [ i ] << "\r\n";
	if ( timeFields.empty ( ) )
		throw std :: runtime_error ( "empty timeFields" );
	for ( unsigned i = 0; i < timeFields.size ( ); i ++ ) {
		const TimeField & f = timeFields [ i ];
		os << "t=" << f.t << "\r\n";
		for ( unsigned j = 0; j < f.r.size ( ); j ++ )
			os << "r=" << f.r [ j ] << "\r\n";
	}
	if ( ! zoneAdjustments.empty ( ) )
		os << zoneAdjustments << "\r\n"; // no letter=
	if ( ! keyField.empty ( ) )
		os << "k=" << keyField << "\r\n";
	for ( AttributeFields :: const_iterator i = attributeFields.begin ( ); i != attributeFields.end ( ); ++ i ) {
		os << "a=" << i -> first;
		if ( ! i -> second.empty ( ) )
			os << ':' << i -> second;
		os << "\r\n";
	}
	for ( MediaDescriptionVector :: const_iterator i = mediaDescriptions.begin ( ); i != mediaDescriptions.end ( ); ++ i )
		i -> printOn ( os );
}

SessionDescription :: Impl :: Impl ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie ) {
	RightParser :: result_t info = parseSdp ( ib, ie );
// Roman next line to remark
//	boost :: spirit :: tree_to_xml ( std :: cout, info.trees );
	if ( ! info.full )
	{
		throw std :: runtime_error ( std :: string ( "error parsing sdp: " ).append ( ib, ie ) );
	}
	RightParser :: iter_t announcement = info.trees.begin ( );
	RightParser :: iter_t line = announcement -> children.begin ( );
	if ( ss :: string ( line -> value.begin ( ), line -> value.end ( ) ) != "0" )
		throw std :: runtime_error ( "unknown sdp proto-version: " +
			std :: string ( line -> value.begin ( ), line -> value.end ( ) ) );
	++ line;
	originField.username.assign ( line -> value.begin ( ), line -> value.end ( ) );
	++ line;
	originField.sessId = std :: strtoull ( ss :: string ( line -> value.begin ( ), line -> value.end ( ) ).c_str ( ), 0, 10 );
	++ line;
	originField.sessVersion = std :: strtoull ( ss :: string ( line -> value.begin ( ), line -> value.end ( ) ).c_str ( ), 0, 10 );
	++ line;
	if ( ss :: string ( line -> value.begin ( ), line -> value.end ( ) ) != "IN" )
		throw std :: runtime_error ( "unsupported sdp nettype: " +
			std :: string ( line -> value.begin ( ), line -> value.end ( ) ) );
	++ line;
	if ( ss :: string ( line -> value.begin ( ), line -> value.end ( ) ) != "IP4" )
		throw std :: runtime_error ( "unsupported sdp addrtype: " +
			std :: string ( line -> value.begin ( ), line -> value.end ( ) ) );
	++ line;
	if ( line -> value.id ( ).to_long ( ) != sdp_grammar_ipv4address_id ) {
		std :: ostringstream os;
		os << "unsupported sdp addr tag: " << line -> value.id ( ).to_long ( );
		throw std :: runtime_error ( os.str ( ) );
	}
	originField.addr.assign ( line -> value.begin ( ), line -> value.end ( ) );
	++ line;
	sessionNameField.assign ( line -> value.begin ( ), line -> value.end ( ) );
	++ line;
	RightParser :: iter_t i = line -> children.begin ( );
	if ( line -> value.id ( ).to_long ( ) == sdp_grammar_information_field_id ) {
		informationField.assign ( i -> value.begin ( ), i -> value.end ( ) );
		++ line;
		i = line -> children.begin ( );
	}
	if ( line -> value.id ( ).to_long ( ) == sdp_grammar_uri_field_id ) {
		uriField.assign ( i -> value.begin ( ), i -> value.end ( ) );
		++ line;
		i = line -> children.begin ( );
	}
	if ( line -> value.id ( ).to_long ( ) == sdp_grammar_email_fields_id ) {
		do
			emailFields.push_back ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
		while ( ++ i != line -> children.end ( ) );
		++ line;
		i = line -> children.begin ( );
	}
	if ( line -> value.id ( ).to_long ( ) == sdp_grammar_phone_fields_id ) {
		do
			phoneFields.push_back ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
		while ( ++ i != line -> children.end ( ) );
		++ line;
		i = line -> children.begin ( );
	}
	if ( line -> value.id ( ).to_long ( ) == sdp_grammar_connection_field_id ) {
		if ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) != "IN" )
			throw std :: runtime_error ( "unsupported sdp nettype: " +
				std :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
		++ i;
		if ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) != "IP4" )
			throw std :: runtime_error ( "unsupported sdp addrtype: " +
				std :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
		++ i;
		connectionField.connectionAddress.assign ( i -> value.begin ( ), i -> value.end ( ) );
		++ line;
		i = line -> children.begin ( );
	}
	if ( line -> value.id ( ).to_long ( ) == sdp_grammar_bandwidth_fields_id ) {
		do {
			ss :: string b ( i -> value.begin ( ), i -> value.end ( ) );
			b += ':';
			++ i;
			b.append ( i -> value.begin ( ), i -> value.end ( ) );
			bandwidthFields.push_back ( b );
		} while ( ++ i != line -> children.end ( ) );
		++ line;
		i = line -> children.begin ( );
	}
	do {
		if ( i -> value.id ( ).to_long ( ) == sdp_grammar_zone_adjustments_id ) {
			zoneAdjustments.assign ( i -> value.begin ( ), i -> value.end ( ) );
			break;
		}
		if ( i -> value.id ( ).to_long ( ) == sdp_grammar_repeat_fields_id )
			timeFields.back ( ).r.push_back ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
		else {
			TimeField t;
			t.t.assign ( i -> value.begin ( ), i -> value.end ( ) );
			t.t += ' ';
			++ i;
			t.t.append ( i -> value.begin ( ), i -> value.end ( ) );
			timeFields.push_back ( t );
		}
	} while ( ++ i != line -> children.end ( ) );
	if ( ++ line == announcement -> children.end ( ) )
		return;
	i = line -> children.begin ( );
	if ( line -> value.id ( ).to_long ( ) == sdp_grammar_key_field_id ) {
		keyField.assign ( i -> value.begin ( ), i -> value.end ( ) );
		if ( ++ line == announcement -> children.end ( ) )
			return;
		i = line -> children.begin ( );
	}
	if ( line -> value.id ( ).to_long ( ) == sdp_grammar_attribute_fields_id ) {
		do {
			RightParser :: iter_t j = i -> children.begin ( );
			AttrType a;
			a.first.assign ( j -> value.begin ( ), j -> value.end ( ) );
			if ( ++ j != i -> children.end ( ) )
				a.second.assign ( j -> value.begin ( ), j -> value.end ( ) );
			attributeFields.push_back ( a );
		} while ( ++ i != line -> children.end ( ) );
		if ( ++ line == announcement -> children.end ( ) )
			return;
		i = line -> children.begin ( );
	}
	RightParser :: iter_t descriptions = line;
	line = i;
	do {
		i = line -> children.begin ( );
		if ( line -> value.id ( ).to_long ( ) == sdp_grammar_information_field_id ) {
			const_cast < MediaDescription & > ( mediaDescriptions.back ( ) ).setInformationField (
				ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
			continue;
		}
		if ( line -> value.id ( ).to_long ( ) == sdp_grammar_connection_field_id ) {
			if ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) != "IN" )
				throw std :: runtime_error ( "unsupported sdp nettype: " +
					std :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
			++ i;
			if ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) != "IP4" )
				throw std :: runtime_error ( "unsupported sdp addrtype: " +
					std :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
			++ i;
			const_cast < MediaDescription & > ( mediaDescriptions.back ( ) ).addConnectionField (
				ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
			continue;
		}
		if ( line -> value.id ( ).to_long ( ) == sdp_grammar_bandwidth_fields_id ) {
			MediaDescription & desc = const_cast < MediaDescription & > ( mediaDescriptions.back ( ) );
			do {
				ss :: string b ( i -> value.begin ( ), i -> value.end ( ) );
				b += ':';
				++ i;
				b.append ( i -> value.begin ( ), i -> value.end ( ) );
				desc.addBandwidthField ( b );
			} while ( ++ i != line -> children.end ( ) );
			continue;
		}
		if ( line -> value.id ( ).to_long ( ) == sdp_grammar_key_field_id ) {
			const_cast < MediaDescription & > ( mediaDescriptions.back ( ) ).setKeyField ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
			continue;
		}
		if ( line -> value.id ( ).to_long ( ) == sdp_grammar_attribute_fields_id ) {
			MediaDescription & desc = const_cast < MediaDescription & > ( mediaDescriptions.back ( ) );
			do {
				RightParser :: iter_t j = i -> children.begin ( );
				ss :: string key ( j -> value.begin ( ), j -> value.end ( ) ), value;
				if ( ++ j != i -> children.end ( ) )
					value.assign ( j -> value.begin ( ), j -> value.end ( ) );
				desc.addAttributeField ( key, value );
			} while ( ++ i != line -> children.end ( ) );
			continue;
		}
		ss :: string media ( i -> value.begin ( ), i -> value.end ( ) );
		++ i;
		unsigned port = std :: atoi ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ).c_str ( ) );
		++ i;
		if ( i -> value.id ( ).to_long ( ) == sdp_grammar_integer_id )
			throw std :: runtime_error ( std :: string ( "unsupported sdp portRange: " ).append (
				i -> value.begin ( ), i -> value.end ( ) ) );
		ss :: string proto ( i -> value.begin ( ), i -> value.end ( ) );
		++ i;
		mediaDescriptions.push_back ( MediaDescription ( media, port, proto ) );
		MediaDescription & desc = const_cast < MediaDescription & > ( mediaDescriptions.back ( ) );
		do
			desc.addFmt ( ss :: string ( i -> value.begin ( ), i -> value.end ( ) ) );
		while ( ++ i != line -> children.end ( ) );
	} while ( ++ line != descriptions -> children.end ( ) );
}
void SessionDescription :: Impl :: setConnectionAddress ( const ss :: string & caddr ) {
	connectionField.connectionAddress = caddr;
}

void SessionDescription :: Impl :: addMediaDescription ( const MediaDescription & m ) {
	mediaDescriptions.push_back ( m );
}

const MediaDescription * SessionDescription :: Impl :: getMediaDescription ( const ss :: string & media ) const {
	typedef MediaDescriptionVector :: index < struct media > :: type ByMedia;
	const ByMedia & byMedia = mediaDescriptions.get < struct media > ( );
	ByMedia :: const_iterator i = byMedia.find ( media );
	if ( i == byMedia.end ( ) )
		return 0;
	return & * i;
}

const ss :: string * SessionDescription :: Impl :: getAttributeValue ( const char * key ) const {
	typedef AttributeFields :: index < attr > :: type ByAttr;
	const ByAttr & byAttr = attributeFields.get < attr > ( );
	ByAttr :: const_iterator i = byAttr.find ( key );
	if ( i == byAttr.end ( ) )
		return 0;
	return & i -> second;
}

void SessionDescription :: Impl :: getAttributeValues ( const char * key, StringVector & v ) const {
	typedef AttributeFields :: index < attr > :: type ByAttr;
	const ByAttr & byAttr = attributeFields.get < attr > ( );
	std :: pair < ByAttr :: const_iterator, ByAttr :: const_iterator > r = byAttr.equal_range ( key );
	for ( ; r.first != r.second; r.first ++ )
		v.push_back ( r.first -> second );
}

const ss :: string & SessionDescription :: Impl :: getConnectionAddress ( ) const {
	return connectionField.connectionAddress;
}

SessionDescription :: SessionDescription ( const ss :: string & s ) : impl ( new Impl ( s ) ) { }

SessionDescription :: SessionDescription ( const ss :: string :: const_iterator & ib,
	const ss :: string :: const_iterator & ie ) : impl ( new Impl ( ib, ie ) ) { }

SessionDescription :: SessionDescription ( const SessionDescription & d ) :
	Allocatable < __SS_ALLOCATOR > ( d ), impl ( new Impl ( * d.impl ) ) { }

SessionDescription & SessionDescription :: operator= ( const SessionDescription & d ) {
	SessionDescription ( d ).swap ( * this );
	return * this;
}

void SessionDescription :: printOn ( std :: ostream & os ) const {
	impl -> printOn ( os );
}

void SessionDescription :: swap ( SessionDescription & d ) {
	std :: swap ( impl, d.impl );
}

SessionDescription :: ~SessionDescription ( ) {
	delete impl;
}

const ss :: string & SessionDescription :: getSessionName ( ) const {
	return impl -> getSessionName ( );
}

void SessionDescription :: setConnectionAddress ( const ss :: string & caddr ) {
	impl -> setConnectionAddress ( caddr );
}

void SessionDescription :: addMediaDescription ( const MediaDescription & m ) {
	impl -> addMediaDescription ( m );
}

const MediaDescription * SessionDescription :: getMediaDescription ( const ss :: string & media ) const {
	return impl -> getMediaDescription ( media );
}

const ss :: string * SessionDescription :: getAttributeValue ( const char * key ) const {
	return impl -> getAttributeValue ( key );
}

void SessionDescription :: getAttributeValues ( const char * key, StringVector & v ) const {
	impl -> getAttributeValues ( key, v );
}

const ss :: string & SessionDescription :: getConnectionAddress ( ) const {
	return impl -> getConnectionAddress ( );
}

}
