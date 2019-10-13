#ifndef __CODECINFO_HPP
#define __CODECINFO_HPP
#pragma interface

#include <boost/optional.hpp>

class CodecInfo : public Allocatable < __SS_ALLOCATOR > {
	ss :: string codec;
	boost :: optional < unsigned > rtpPayload;
	int frames;
	bool canRecode, supported;
public:
	CodecInfo ( const char * c, int f = 0, bool r = false, bool s = true );
	CodecInfo ( const ss :: string & c = "unknown", int f = 0, bool r = false, bool s = true );
	CodecInfo ( unsigned p, const ss :: string & c, int f );
	~CodecInfo ( ) { }
	const ss :: string & getCodec ( ) const;
	int getFrames ( ) const;
	void setFrames ( int f );
	bool getCanRecode ( ) const;
	bool getSupported ( ) const;
	void setCodec ( const ss :: string & c );
	CodecInfo & setPayload ( unsigned p );
	const boost :: optional < unsigned > & getPayload ( ) const;
	static bool isTrivialRecoder ( const ss :: string & from, const ss :: string & to );
};

bool isDifferentCodec ( const CodecInfo & c1, const CodecInfo & c2 );

typedef std :: set < CodecInfo, std :: less < CodecInfo >, __SS_ALLOCATOR < CodecInfo > > CodecInfoSet;
CodecInfoSet :: const_iterator findRecode ( const CodecInfo & codec, const CodecInfoSet & allowedCodecs,
	const StringStringSetMap & recodes );
//typedef std :: vector < CodecInfo, __SS_ALLOCATOR < CodecInfo > > CodecInfoVector;
struct Codec { };
typedef boost :: multi_index :: multi_index_container < CodecInfo,
	boost :: multi_index :: indexed_by < boost :: multi_index :: sequenced < >,
	boost :: multi_index :: ordered_unique < boost :: multi_index :: tag < Codec >,
	boost :: multi_index :: identity < CodecInfo > > >,
	__SS_ALLOCATOR < CodecInfo > > CodecInfoContainer;
struct CodecInfoVector {
	CodecInfoContainer c;
};
bool operator< ( const CodecInfo & c1, const CodecInfo & c2 );
bool operator== ( const CodecInfo & c1, const CodecInfo & c2 );
std :: ostream & operator<< ( std :: ostream & os, const CodecInfo & c );

#endif
