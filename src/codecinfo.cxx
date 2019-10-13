#pragma implementation
#include "ss.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "allocatable.hpp"
#include "codecinfo.hpp"

CodecInfo :: CodecInfo ( const ss :: string & c, int f, bool r, bool s ) : codec ( c ), frames ( f ), canRecode ( r ),
	supported ( s ) { }

CodecInfo :: CodecInfo ( const char * c, int f, bool r, bool s ) : codec ( c ), frames ( f ), canRecode ( r ),
	supported ( s ) { }

CodecInfo :: CodecInfo ( unsigned p, const ss :: string & c, int f ) : codec ( c ), rtpPayload ( p ), frames ( f ),
	canRecode ( false ), supported ( true ) { }

const ss :: string & CodecInfo :: getCodec ( ) const {
	return codec;
}

void CodecInfo :: setCodec ( const ss :: string & c ) {
	codec = c;
}

int CodecInfo :: getFrames ( ) const {
	return frames;
}

void CodecInfo :: setFrames ( int f ) {
	frames = f;
}

bool CodecInfo :: getCanRecode ( ) const {
	return canRecode;
}

bool CodecInfo :: getSupported ( ) const {
	return supported;
}

CodecInfo & CodecInfo :: setPayload ( unsigned p ) {
	rtpPayload = p;
	return * this;
}

const boost :: optional < unsigned > & CodecInfo :: getPayload ( ) const {
	return rtpPayload;
}

bool CodecInfo :: isTrivialRecoder ( const ss :: string & from, const ss :: string & to ) {
	static const char * g729names [ ] = { "g729r8", "g729ar8", "g729br8", "g729abr8" };
	static const StringSet g729set ( g729names, g729names + sizeof ( g729names ) / sizeof ( * g729names ) );
	return g729set.count ( from ) && g729set.count ( to );
}

bool isDifferentCodec ( const CodecInfo & c1, const CodecInfo & c2 ) {
	if ( c1 == c2 )
		return false;
	if ( CodecInfo :: isTrivialRecoder ( c1.getCodec ( ), c2.getCodec ( ) ) )
		return false;
	return true;
}

CodecInfoSet :: const_iterator findRecode ( const CodecInfo & codec, const CodecInfoSet & allowedCodecs,
	const StringStringSetMap & recodes ) {
	StringStringSetMap :: const_iterator j = recodes.find ( codec.getCodec ( ) );
	if ( j == recodes.end ( ) )
		return allowedCodecs.end ( );
	for ( StringSet :: const_iterator k = j -> second.begin ( ); k != j -> second.end ( ); ++ k ) {
		CodecInfoSet :: const_iterator i = allowedCodecs.find ( * k );
		if ( i != allowedCodecs.end ( ) && i -> getCanRecode ( ) )
			return i;
	}
	return allowedCodecs.end ( );
}

bool operator< ( const CodecInfo & c1, const CodecInfo & c2 ) {
	return c1.getCodec ( ) < c2.getCodec ( );
}

bool operator== ( const CodecInfo & c1, const CodecInfo & c2 ) {
	return c1.getCodec ( ) == c2.getCodec ( );
}

std :: ostream & operator<< ( std :: ostream & os, const CodecInfo & c ) {
	os << c.getCodec ( ) << ':';
	if ( const boost :: optional < unsigned > & p = c.getPayload ( ) )
		os << * p;
	os << ':' << c.getFrames ( ) << ':' << c.getCanRecode ( ) << ':' << c.getSupported ( );
	return os;
}
