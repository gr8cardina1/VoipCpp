#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include "sdp.hpp"
#include "codecinfo.hpp"
#include "sdpcommon.hpp"
#include <tr1/functional>
#include "findmapelement.hpp"
#include "ssconf.hpp"
#include <ptlib.h>
#include <ptlib/svcproc.h>
#include <stdexcept>
#include <boost/algorithm/string/case_conv.hpp>

typedef std :: tr1 :: function < CodecInfo ( unsigned, unsigned, const ss :: string & ) > CreateCodec;
typedef std :: map < ss :: string, CreateCodec, std :: less < ss :: string >,
	__SS_ALLOCATOR < std :: pair < const ss :: string, CreateCodec > > > CodecMap;
static CodecInfo identityCodec ( const ss :: string & name, unsigned frameLen, unsigned defaultFrames, unsigned payload,
	unsigned ptime, const ss :: string & /*fmtp*/ ) {
	return CodecInfo ( payload, name, ptime ? ptime / frameLen : defaultFrames );
}

static CodecInfo decode723 ( unsigned payload, unsigned ptime, const ss :: string & fmtp ) {
	if ( fmtp.find ( "bitrate=5300" ) != ss :: string :: npos )
		return CodecInfo ( payload, "g723ar53", ( ptime ? : 30 ) / 30 );
	else if ( fmtp.find ( "bitrate=6300" ) != ss :: string :: npos )
		return CodecInfo ( payload, "g723ar63", ( ptime ? : 30 ) / 30 );
	else if ( fmtp.find ( "annexa=no" ) != ss :: string :: npos )
		return CodecInfo ( payload, "g7231", ( ptime ? : 30 ) / 30 );
	else
		return CodecInfo ( payload, "g723ar", ( ptime ? : 30 ) / 30 );
}

static CodecInfo decode729 ( unsigned payload, unsigned ptime, const ss :: string & fmtp ) {
	if ( noAnnexB || fmtp.find ( "annexb=no" ) != ss :: string :: npos )
		return CodecInfo ( payload, "g729r8", ( ptime ? : 20 ) / 10 );
	else
		return CodecInfo ( payload, "g729br8", ( ptime ? : 20 ) / 10 );
}


static CodecMap encodingCodecMap ( ) {
	CodecMap m;
	using std :: tr1 :: placeholders :: _1;
	using std :: tr1 :: placeholders :: _2;
	using std :: tr1 :: placeholders :: _3;
	m [ "ILBC" ] = std :: tr1 :: bind ( identityCodec, "ilbc", 1000, 0, _1, _2, _3 );
	m [ "TELEPHONE-EVENT" ] = std :: tr1 :: bind ( identityCodec, "telephone-event", 1000, 0, _1, _2, _3 );
	m [ "GSM" ] = std :: tr1 :: bind ( identityCodec, "gsmfr", 20, 0, _1, _2, _3 );
	m [ "GSM-EFR" ] = std :: tr1 :: bind ( identityCodec, "gsmefr", 20, 0, _1, _2, _3 );
	m [ "G726-16" ] = std :: tr1 :: bind ( identityCodec, "g726r16", 1000, 0, _1, _2, _3 );
	m [ "G726-24" ] = std :: tr1 :: bind ( identityCodec, "g726r24", 1000, 0, _1, _2, _3 );
	m [ "G726-32" ] = std :: tr1 :: bind ( identityCodec, "g726r32", 1000, 0, _1, _2, _3 );
	m [ "G726" ] = std :: tr1 :: bind ( identityCodec, "g726", 1000, 0, _1, _2, _3 );
	m [ "G721" ] = std :: tr1 :: bind ( identityCodec, "g721", 1000, 0, _1, _2, _3 );
	m [ "G723" ] = decode723;
	m [ "PCMU" ] = std :: tr1 :: bind ( identityCodec, "g711ulaw", 1, 20, _1, _2, _3 );
	m [ "PCMA" ] = std :: tr1 :: bind ( identityCodec, "g711alaw", 1, 20, _1, _2, _3 );
	m [ "G729" ] = decode729;
	return m;
}

static CodecMap payloadCodecMap ( ) {
	CodecMap m;
	using std :: tr1 :: placeholders :: _1;
	using std :: tr1 :: placeholders :: _2;
	using std :: tr1 :: placeholders :: _3;
	m [ SDP :: G729 ] = decode729;
	m [ SDP :: G7231 ] = decode723;
	m [ SDP :: G726 ] = std :: tr1 :: bind ( identityCodec, "g726", 1000, 0, _1, _2, _3 );
	m [ SDP :: PCMA ] = std :: tr1 :: bind ( identityCodec, "g711alaw", 1, 2, _1, _2, _3 );
	m [ SDP :: PCMU ] = std :: tr1 :: bind ( identityCodec, "g711ulaw", 1, 2, _1, _2, _3 );
	m [ SDP :: G728 ] = std :: tr1 :: bind ( identityCodec, "g728", 1000, 0, _1, _2, _3 );
	m [ SDP :: GSM ] = std :: tr1 :: bind ( identityCodec, "gsmfr", 20, 0, _1, _2, _3 );
	return m;
}

CodecInfo getCodec ( const SDP :: MediaFormat & format, unsigned ptime ) {
	ss :: string encoding ( format.getRtpmap ( ), 0, format.getRtpmap ( ).find ( '/' ) );
	unsigned payload = std :: atoi ( format.getFmt ( ).c_str ( ) );
	if ( ! encoding.empty ( ) ) {
		boost :: algorithm :: to_upper ( encoding );
		static CodecMap m = encodingCodecMap ( );
		if ( CreateCodec * f = findMapElement ( m, encoding ) )
			return ( * f ) ( payload, ptime, format.getFmtp ( ) );
	}
	static CodecMap m = payloadCodecMap ( );
	if ( CreateCodec * f = findMapElement ( m, format.getFmt ( ) ) )
		return ( * f ) ( payload, ptime, format.getFmtp ( ) );
	PSYSTEMLOG ( Error, "unsupported format: " << format );
	return CodecInfo ( payload, "unknown", 0 );
}

static ss :: string getPayload ( const CodecInfo & c, const ss :: string & def ) {
	if ( ! c.getPayload ( ) )
		return def;
	ss :: ostringstream os;
	os << * c.getPayload ( );
	return os.str ( );
}

static void setPtime ( SDP :: MediaDescription & media, unsigned ptime ) {
	if ( media.getFmtsSize ( ) <= 1 ) {
		ss :: ostringstream os;
		os << ptime;
		media.addAttributeField ( "ptime", os.str ( ) );
	}
}

ss :: string addFormat ( SDP :: MediaDescription & media, const CodecInfo & c ) {
	if ( c.getCodec ( ) == "g729r8" || c.getCodec ( ) == "g729ar8" || c.getCodec ( ) == "g729" ) {
		ss :: string payload = getPayload ( c, SDP :: G729 );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " G729/8000" );
		media.addAttributeField ( "fmtp", payload + " annexb=no" );
		if ( c.getFrames ( ) )
			setPtime ( media, c.getFrames ( ) * 10 );
		return payload;
	}
	if ( c.getCodec ( ) == "g729br8" || c.getCodec ( ) == "g729abr8" ) {
		ss :: string payload = getPayload ( c, SDP :: G729 );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " G729/8000" );
		if ( c.getFrames ( ) )
			setPtime ( media, c.getFrames ( ) * 10 );
		return payload;
	}
	if ( c.getCodec ( ) == "g723ar" ) {
		ss :: string payload = getPayload ( c, SDP :: G7231 );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " G723/8000" );
		if ( c.getFrames ( ) )
			setPtime ( media, c.getFrames ( ) * 30 );
		return payload;
	}
	if ( c.getCodec ( ) == "g723ar53" ) {
		ss :: string payload = getPayload ( c, SDP :: G7231 );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " G723/8000" );
		media.addAttributeField ( "fmtp", payload + " bitrate=5300" );
		if ( c.getFrames ( ) )
			setPtime ( media, c.getFrames ( ) * 30 );
		return payload;
	}
	if ( c.getCodec ( ) == "g723ar63" ) {
		ss :: string payload = getPayload ( c, SDP :: G7231 );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " G723/8000" );
		media.addAttributeField ( "fmtp", payload + " bitrate=6300" );
		if ( c.getFrames ( ) )
			setPtime ( media, c.getFrames ( ) * 30 );
		return payload;
	}
	if ( c.getCodec ( ) == "g7231" ) {
		ss :: string payload = getPayload ( c, SDP :: G7231 );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " G723/8000" );
		media.addAttributeField ( "fmtp", payload + " annexa=no" );
		if ( c.getFrames ( ) )
			setPtime ( media, c.getFrames ( ) * 30 );
		return payload;
	}
	if ( c.getCodec ( ) == "g726" ) {
		ss :: string payload = getPayload ( c, SDP :: G726 );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " G726/8000" );
		return payload;
	}
	if ( c.getCodec ( ) == "g726r16" ) {
		ss :: string payload = getPayload ( c, SDP :: G726 );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " G726-16/8000" );
		return payload;
	}
	if ( c.getCodec ( ) == "g726r24" ) {
		ss :: string payload = getPayload ( c, SDP :: G726 );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " G726-24/8000" );
		return payload;
	}
	if ( c.getCodec ( ) == "g726r32" ) {
		ss :: string payload = getPayload ( c, SDP :: G726 );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " G726-32/8000" );
		return payload;
	}
	if ( c.getCodec ( ) == "g711alaw" ) {
		ss :: string payload = getPayload ( c, SDP :: PCMA );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " PCMA/8000" );
		if ( c.getFrames ( ) )
			setPtime ( media, c.getFrames ( ) );
		return payload;
	}
	if ( c.getCodec ( ) == "g711ulaw" ) {
		ss :: string payload = getPayload ( c, SDP :: PCMU );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " PCMU/8000" );
		if ( c.getFrames ( ) )
			setPtime ( media, c.getFrames ( ) );
		return payload;
	}
	if ( c.getCodec ( ) == "g728" ) {
		ss :: string payload = getPayload ( c, SDP :: G728 );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " G728/8000" );
		if ( c.getFrames ( ) )
			setPtime ( media, c.getFrames ( ) * 5 / 2 );
		return payload;
	}
	if ( c.getCodec ( ) == "gsmfr" ) {
		ss :: string payload = getPayload ( c, SDP :: GSM );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " GSM/8000" );
		if ( c.getFrames ( ) )
			setPtime ( media, c.getFrames ( ) * 20 );
		return payload;
	}
	if ( c.getCodec ( ) == "gsmefr" ) {
		ss :: string payload = getPayload ( c, SDP :: GSM );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " GSM-EFR/8000" );
		if ( c.getFrames ( ) )
			setPtime ( media, c.getFrames ( ) * 20 );
		return payload;
	}
	if ( c.getCodec ( ) == "ilbc" ) {
		if ( ! c.getPayload ( ) )
			throw std :: runtime_error ( "no payload for ilbc" );
		ss :: ostringstream os;
		os << * c.getPayload ( );
		ss :: string payload = os.str ( );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " iLBC/8000" );
		return payload;
	}
	if ( c.getCodec ( ) == "telephone-event" ) {
		ss :: string payload = getPayload ( c, SDP :: TelephoneEvent );
		media.addFmt ( payload );
		media.addAttributeField ( "rtpmap", payload + " telephone-event/8000" );
		media.addAttributeField ( "fmtp", payload + " 0-15" );
		return payload;
	}
	std :: ostringstream os;
	os << "unsupported sdp codec: " << c;
	throw std :: runtime_error ( os.str ( ) );
}

void addTelephoneEvents ( SDP :: MediaDescription & media, unsigned payloadType ) {
	ss :: ostringstream os;
	os << payloadType;
	ss :: string payload = os.str ( );
	media.addFmt ( payload );
	media.addAttributeField ( "rtpmap", payload + " telephone-event/8000" );
	media.addAttributeField ( "fmtp", payload + " 0-15" );
}

unsigned getTelephoneEventsPayloadType ( const SDP :: MediaDescription * media ) {
	typedef SDP :: MediaFormatVector :: index < SDP :: rtpmap > :: type ByRtpmap;
	const ByRtpmap & byRtpmap = media -> getFmts ( ).get < SDP :: rtpmap > ( );
	ByRtpmap :: const_iterator i = byRtpmap.lower_bound ( "telephone-event" );
	if ( i == byRtpmap.end ( ) )
		return 0;
	return std :: atoi ( i -> getFmt ( ).c_str ( ) );
}

static void parseCdsc ( const StringVector & cdsc, unsigned ptime, CodecInfoVector & incomingCodecs ) {
	for ( unsigned i = 0; i < cdsc.size ( ); i ++ ) {
		ss :: istringstream is ( cdsc [ i ] );
		ss :: string s;
		is >> s >> s >> s;
		while ( is >> s ) {
			CodecInfo c = getCodec ( SDP :: MediaFormat ( s ), ptime );
			if ( c.getCodec ( ) != "unknown" && c.getCodec ( ) != "telephone-event" )
				incomingCodecs.c.push_back ( c );
		}
	}
}

void getIncomingCodecsFromSDP ( const SDP :: SessionDescription & sdp, CodecInfoVector & incomingCodecs ) {
	const SDP :: MediaDescription * media = sdp.getMediaDescription ( SDP :: mediaAudio );
	unsigned ptime = 0;
	if ( media ) {
		ptime = getPtime ( media );
		const SDP :: MediaFormatVector & formats = media -> getFmts ( );
		for ( SDP :: MediaFormatVector :: const_iterator i = formats.begin ( ); i != formats.end ( ); ++ i ) {
			const SDP :: MediaFormat & f = * i;
			CodecInfo c = getCodec ( f, ptime );
			if ( c.getCodec ( ) != "unknown" && c.getCodec ( ) != "telephone-event" )
				incomingCodecs.c.push_back ( c );
		}
	}
	StringVector cdsc;
	sdp.getAttributeValues ( "cdsc", cdsc );
	parseCdsc ( cdsc, 0, incomingCodecs );
	if ( media ) {
		cdsc.clear ( );
		media -> getAttributeValues ( "cdsc", cdsc );
		parseCdsc ( cdsc, ptime, incomingCodecs );
	}
}

