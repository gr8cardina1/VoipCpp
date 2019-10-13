#ifndef SLPRINT_HPP_
#define SLPRINT_HPP_
#pragma interface

class SLPrint {
	ss :: string header;
public:
	explicit SLPrint ( const ss :: string & h ) : header ( h ) { }
	template < class T > void operator() ( const T & t ) {
		PSYSTEMLOG ( Info, header << t );
	}
};

class SLMapPrint {
	ss :: string header;
public:
	explicit SLMapPrint ( const ss :: string & h ) : header ( h ) { }
	void operator() ( const StringStringSetMap :: value_type & t ) {
		ss :: ostringstream os;
		os << header << t.first << " -> ";
		std :: for_each ( t.second.begin ( ), t.second.end ( ), SLPrint ( os.str ( ) ) );
	}
};

template < typename T1, typename T2 > std :: ostream & operator<< ( std :: ostream & os, const std :: pair < T1, T2 > & p ) {
	return os << '(' << p.first << ' ' << p.second << ')';
}

namespace ss {
	template < typename _Range, typename _Function > void for_each ( const _Range & r, const _Function & f ) {
		//prototype otlichaetsya ot standartnogo potomu chto inache budet slishkom mnogo copy constructorov vizvano
		std :: for_each ( boost :: begin ( r ), boost :: end ( r ), f );
	}
}
#endif /*SLPRINT_HPP_*/
