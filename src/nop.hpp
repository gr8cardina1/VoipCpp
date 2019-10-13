#ifndef NOP_HPP_
#define NOP_HPP_
#pragma interface

struct Nop {
	template < typename T > void operator() ( T ) { }
};

#endif /*NOP_HPP_*/
