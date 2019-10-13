#pragma implementation
#include "ss.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "allocatable.hpp"
#include "codecinfo.hpp"

#include "sipcommon.hpp"
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "ixcudpsocket.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include "sip.hpp"
#include <ptlib/svcproc.h>
#include "ssconf.hpp"

static CodecInfo decode723 ( const SIP :: MediaFormat & format ) {
	if ( format.getFMTP ( ).find ( "bitrate=5300" ) != ss :: string :: npos )
		return CodecInfo ( "g723ar53", ( format.getPtime ( ) ? : 30 ) / 30 );
	else if ( format.getFMTP ( ).find ( "bitrate=6300" ) != ss :: string :: npos )
		return CodecInfo ( "g723ar63", ( format.getPtime ( ) ? : 30 ) / 30 );
	else if ( format.getFMTP ( ).find ( "annexa=no" ) != ss :: string :: npos )
		return CodecInfo ( "g7231", ( format.getPtime ( ) ? : 30 ) / 30 );
	else
		return CodecInfo ( "g723ar", ( format.getPtime ( ) ? : 30 ) / 30 );
}

static CodecInfo decodePcmu ( const SIP :: MediaFormat & format ) {
	return CodecInfo ( "g711ulaw", format.getPtime ( ) ? : 2 );
}

static CodecInfo decodePcma ( const SIP :: MediaFormat & format ) {
	return CodecInfo ( "g711alaw", format.getPtime ( ) ? : 2 );
}

static CodecInfo decode729 ( const SIP :: MediaFormat & format ) {
	if ( noAnnexB || format.getFMTP ( ).find ( "annexb=no" ) != ss :: string :: npos )
		return CodecInfo ( "g729r8", ( format.getPtime ( ) ? : 20 ) / 10 );
	else
		return CodecInfo ( "g729br8", ( format.getPtime ( ) ? : 20 ) / 10 );
}

static CodecInfo getCodecInt ( const SIP :: MediaFormat & format ) {
	if ( format.getEncodingName ( ) == "iLBC" || format.getEncodingName ( ) == "ilbc" )
		return "iLBC";
	if ( format.getEncodingName ( ) == "telephone-event" )
		return "telephone-event";
	if ( format.getEncodingName ( ) == "GSM" )
		return "gsmfr";
	if ( format.getEncodingName ( ) == "GSM-EFR" )
		return "gsmefr";
	if ( format.getEncodingName ( ) == "G726-16" )
		return "g726r16";
	if ( format.getEncodingName ( ) == "G726-24" )
		return "g726r24";
	if ( format.getEncodingName ( ) == "G726-32" )
		return "g726r32";
	if ( format.getEncodingName ( ) == "G726" )
		return "g726";
	if ( format.getEncodingName ( ) == "G721" )
		return "g721";
	if ( format.getEncodingName ( ) == "G723" )
		return decode723 ( format );
	if ( format.getEncodingName ( ) == "PCMU" )
		return decodePcmu ( format );
	if ( format.getEncodingName ( ) == "PCMA" )
		return decodePcma ( format );
	if ( format.getEncodingName ( ) == "G729" )
		return decode729 ( format );
	switch ( format.getPayloadType ( ) ) {
		case SIP :: MediaFormat :: G729:
			return decode729 ( format );
		case SIP :: MediaFormat :: G7231:
			return decode723 ( format );
		case SIP :: MediaFormat :: G726:
			return "g726";
		case SIP :: MediaFormat :: PCMA:
			return decodePcma ( format );
		case SIP :: MediaFormat :: PCMU:
			return decodePcmu ( format );
		case SIP :: MediaFormat :: G728:
			return "g728";
		case SIP :: MediaFormat :: GSM:
			return "gsmfr";
		default:;
	}
	PSYSTEMLOG ( Error, "unsupported format: " << format );
	return "unknown";
}

CodecInfo getCodec ( const SIP :: MediaFormat & format ) {
	CodecInfo c = getCodecInt ( format );
	c.setPayload ( format.getPayloadType ( ) );
	return c;
}

ss :: string toHex ( const ss :: string & s ) {
	ss :: ostringstream os;
	typedef unsigned char uchar;
	for ( unsigned i = 0; i < s.size ( ); i ++ )
		os << std :: setw ( 2 ) << std :: setfill ( '0' ) << std :: hex << unsigned ( uchar ( s [ i ] ) );
	return os.str ( );
}

bool getIpFromVia ( const ss :: string & via, ss :: string & ip ) {
	ss :: string :: size_type i1 = via.find ( ' ' );
	if ( i1 == ss :: string :: npos )
		return false;
	i1 ++;
	ss :: string :: size_type i2 = via.find_first_of ( ";:", i1 );
	ip = via.substr ( i1, i2 - i1 );
	return true;
}

bool getPortFromVia ( const ss :: string & via, int & port ) {
	ss :: string :: size_type i1 = via.find_first_of ( ";:" );
	if ( i1 == ss :: string :: npos || via [ i1 ] == ';' )
		return false;
	i1 ++;
	ss :: istringstream is ( via.substr ( i1 ) );
	is >> port;
	return true;
}

void getIncomingCodecsFromSDP ( const SIP :: SessionDescription & sdp, CodecInfoVector & incomingCodecs ) {
	if ( const SIP :: MediaDescription * media = sdp.getMediaDescription ( SIP :: MediaDescription :: mtAudio ) ) {
		const SIP :: MediaFormatVector & formats = media -> getMediaFormats ( );
		for ( SIP :: MediaFormatVector :: const_iterator i = formats.begin ( ); i != formats.end ( ); ++ i ) {
			CodecInfo c = getCodec ( * i );
			if ( c.getCodec ( ) != "unknown" && c.getCodec ( ) != "telephone-event" ) {
				PSYSTEMLOG ( Info, "getIncomingCodecsFromSDP: added codec " << c.getCodec ( ) );
				incomingCodecs.c.push_back ( c );
			}
		}
	}
}

template < class T, class U > static const T get ( const boost :: optional < T > & o, U d ) {
	if ( o )
		return * o;
	return d;
}

void addFormat ( SIP :: MediaDescription & media, const CodecInfo & c ) {
	if ( c.getCodec ( ) == "g729r8" || c.getCodec ( ) == "g729ar8" || c.getCodec ( ) == "g729" ) {
		SIP :: MediaFormat f ( get ( c.getPayload ( ), SIP :: MediaFormat :: G729 ), "G729", 8000, "", "annexb=no" );
		if ( c.getFrames ( ) )
			f.setPtime ( c.getFrames ( ) * 10 );
		media.addMediaFormat ( f );
	} else if ( c.getCodec ( ) == "g729br8" || c.getCodec ( ) == "g729abr8" ) {
//		SIP :: MediaFormat f ( SIP :: MediaFormat :: G729, "G729", 8000, "", "annexb=yes" );
		SIP :: MediaFormat f ( get ( c.getPayload ( ), SIP :: MediaFormat :: G729 ), "G729" );
		if ( c.getFrames ( ) )
			f.setPtime ( c.getFrames ( ) * 10 );
		media.addMediaFormat ( f );
	} else if ( c.getCodec ( ) == "g723ar" ) {
		SIP :: MediaFormat f ( get ( c.getPayload ( ), SIP :: MediaFormat :: G7231 ), "G723" );
		if ( c.getFrames ( ) )
			f.setPtime ( c.getFrames ( ) * 30 );
		media.addMediaFormat ( f );
	} else if ( c.getCodec ( ) == "g723ar53" ) {
		SIP :: MediaFormat f ( get ( c.getPayload ( ), SIP :: MediaFormat :: G7231 ), "G723", 8000, "",
			"bitrate=5300" );
		if ( c.getFrames ( ) )
			f.setPtime ( c.getFrames ( ) * 30 );
		media.addMediaFormat ( f );
	} else if ( c.getCodec ( ) == "g723ar63" ) {
		SIP :: MediaFormat f ( get ( c.getPayload ( ), SIP :: MediaFormat :: G7231 ), "G723", 8000, "",
			"bitrate=6300" );
		if ( c.getFrames ( ) )
			f.setPtime ( c.getFrames ( ) * 30 );
		media.addMediaFormat ( f );
	} else if ( c.getCodec ( ) == "g7231" ) {
		SIP :: MediaFormat f ( get ( c.getPayload ( ), SIP :: MediaFormat :: G7231 ), "G723", 8000, "", "annexa=no" );
		if ( c.getFrames ( ) )
			f.setPtime ( c.getFrames ( ) * 30 );
		media.addMediaFormat ( f );
	} else if ( c.getCodec ( ) == "g726" )
		media.addMediaFormat ( SIP :: MediaFormat ( get ( c.getPayload ( ), SIP :: MediaFormat :: G726 ), "G726" ) );
	else if ( c.getCodec ( ) == "g726r16" )
		media.addMediaFormat ( SIP :: MediaFormat ( get ( c.getPayload ( ), SIP :: MediaFormat :: G726 ), "G726-16" ) );
	else if ( c.getCodec ( ) == "g726r24" )
		media.addMediaFormat ( SIP :: MediaFormat ( get ( c.getPayload ( ), SIP :: MediaFormat :: G726 ), "G726-24" ) );
	else if ( c.getCodec ( ) == "g726r32" )
		media.addMediaFormat ( SIP :: MediaFormat ( get ( c.getPayload ( ), SIP :: MediaFormat :: G726 ), "G726-32" ) );
	else if ( c.getCodec ( ) == "g711alaw" ) {
		SIP :: MediaFormat f ( get ( c.getPayload ( ), SIP :: MediaFormat :: PCMA ), "PCMA" );
		if ( c.getFrames ( ) )
		{	// f.setPtime ( c.getFrames ( ) * 10 );// bug #1993
			f.setPtime ( c.getFrames ( ) );
		}
		media.addMediaFormat ( f );
	} else if ( c.getCodec ( ) == "g711ulaw" ) {
		SIP :: MediaFormat f ( get ( c.getPayload ( ), SIP :: MediaFormat :: PCMU ), "PCMU" );
		if ( c.getFrames ( ) )
		{	// f.setPtime ( c.getFrames ( ) * 10 );// bug #1993
			f.setPtime ( c.getFrames ( ) );
		}
		media.addMediaFormat ( f );
	} else if ( c.getCodec ( ) == "g728" )
		media.addMediaFormat ( SIP :: MediaFormat ( get ( c.getPayload ( ), SIP :: MediaFormat :: G728 ), "G728" ) );
	else if ( c.getCodec ( ) == "gsmfr" )
		media.addMediaFormat ( SIP :: MediaFormat ( get ( c.getPayload ( ), SIP :: MediaFormat :: GSM ), "GSM" ) );
	else if ( c.getCodec ( ) == "gsmefr" )
		media.addMediaFormat ( SIP :: MediaFormat ( get ( c.getPayload ( ), SIP :: MediaFormat :: GSM ), "GSM-EFR" ) );
	else if ( c.getCodec ( ) == "telephone-event" )
		media.addMediaFormat ( SIP :: MediaFormat ( media.getTelephoneEventsPayloadType ( ),
			"telephone-event" ).setFMTP ( "0-15" ) );
	else
		PSYSTEMLOG ( Error, "unsupported sip codec: " << c );
}

ss :: string changeCSeq ( const ss :: string & oldCSeq, const ss :: string & method ) {
	ss :: istringstream is ( oldCSeq );
	ss :: string num;
	is >> num;
	return num + " " + method;
}

unsigned allocTelephoneEventsPayload ( const SIP :: MediaDescriptionVector & mdv ) {
	unsigned usedSet = 0;
	for ( SIP :: MediaDescriptionVector :: const_iterator cit = mdv.begin ( ); cit != mdv.end ( ); ++ cit ) {
		const SIP :: MediaFormatVector & mfv = cit -> getMediaFormats ( );
		for ( SIP :: MediaFormatVector :: const_iterator it = mfv.begin ( ); it != mfv.end ( ); ++ it ) {
			unsigned t = it -> getPayloadType ( ) - SIP :: MediaFormat :: DynamicBase;
			if ( t < SIP :: MediaFormat :: MaxPayloadType - SIP :: MediaFormat :: DynamicBase )
				usedSet |= 1 << t;
		}
	}
	if ( ( usedSet & ( 1 << ( 101 - SIP :: MediaFormat :: DynamicBase ) ) ) == 0 )
		return 101;
	unsigned freeSet = ~ usedSet;
	unsigned t = __builtin_ctz ( freeSet ) + SIP :: MediaFormat :: DynamicBase;
	if ( __builtin_expect ( t == SIP :: MediaFormat :: MaxPayloadType, 0 ) )
		t = 0;
	return t;
}

void clearContent ( SIP :: PDU & mesg ) {
	mesg.setSDP ( 0 );
	mesg.setEntityBody ( "" );
	SIP :: MIMEInfo & mime = mesg.getMIME ( );
	mime.setContentLength ( 0 );
	mime.setContentType ( "" );
}

