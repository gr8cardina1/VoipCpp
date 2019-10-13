#ifndef ISTRINGLESS_HPP_
#define ISTRINGLESS_HPP_
#pragma interface

template < class _String > class istringless : public std :: binary_function < _String, _String, bool > {
	typedef typename _String :: value_type char_type;
	class CharLess : public std :: binary_function < char_type, char_type, bool > {
		const std :: ctype < char_type > & m_ctype;
	public:
		CharLess ( const std :: ctype < char_type > & c ) : m_ctype ( c ) { }
		bool operator() ( const char_type & lhs, const char_type & rhs ) const {
			return m_ctype.tolower ( lhs ) < m_ctype.tolower ( rhs );
		}
	};
	const std :: locale m_loc;
	const std :: ctype < char_type > & m_ctype;
public:
	istringless ( const std :: locale & l = std :: locale ( ) ) : m_loc ( l ),
		m_ctype ( std :: use_facet < std :: ctype < char_type > > ( m_loc ) ) { }
	bool operator() ( const _String & lhs, const _String & rhs ) const {
		return std :: lexicographical_compare ( lhs.begin ( ), lhs.end ( ), rhs.begin ( ), rhs.end ( ),
			CharLess ( m_ctype ) );
	}
};

#endif /*ISTRINGLESS_HPP_*/
