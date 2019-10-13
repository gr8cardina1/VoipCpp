#ifndef RIGHTPARSER_HPP_
#define RIGHTPARSER_HPP_
#pragma interface

namespace boost {
namespace spirit {
namespace classic {

template < typename NodeFactoryT, typename IteratorT, typename ParserT > inline tree_parse_info < IteratorT,
	NodeFactoryT > pt_parse ( const IteratorT & first_, const IteratorT & last, const parser < ParserT > & p,
	const NodeFactoryT & /*dummy_*/ = NodeFactoryT ( ) ) {
	typedef pt_match_policy < IteratorT, NodeFactoryT > pt_match_policy_t;
	IteratorT first = first_;
	scanner < IteratorT, scanner_policies < iteration_policy, pt_match_policy_t > > scan ( first, last );
	tree_match < IteratorT, NodeFactoryT > hit = p.derived ( ).parse ( scan );
	return tree_parse_info < IteratorT, NodeFactoryT > ( first, hit, hit && first == last, hit.length ( ),
		hit.trees );
}

}
}
}

struct RightParser {
private:
	typedef boost :: spirit :: classic :: node_iter_data_factory < > factory_t;
	typedef ss :: string :: const_iterator iterator_t;
	typedef boost :: spirit :: classic :: tree_match < iterator_t, factory_t > parse_tree_match_t;
public:
	typedef parse_tree_match_t :: const_tree_iterator iter_t;
	typedef boost :: spirit :: classic :: tree_parse_info < iterator_t, factory_t > result_t;
	template < class ParserT > static result_t parse ( const iterator_t & first, const iterator_t & last,
		const boost :: spirit :: classic :: parser < ParserT > & parser ) {
		return boost :: spirit :: classic :: pt_parse < factory_t > ( first, last, parser );
	}
};

#endif /*RIGHTPARSER_HPP_*/
