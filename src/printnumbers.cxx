#include "ss.hpp"
#include <iomanip>
#include <limits>

static void printNumbersOn ( std :: ostream & os, const ss :: string & s, ss :: string :: size_type sSize ) {
	unsigned line_width = unsigned ( os.width ( ) );
	unsigned size = sizeof ( s [ 0 ] );
	if ( line_width == 0 )
		line_width = 16 / size;
	os.width ( 0 );
	unsigned indent = unsigned ( os.precision ( ) );
	unsigned val_width;
	bool is_signed = false;
	switch ( os.flags ( ) & std :: ios :: basefield ) {
		case std :: ios :: hex:
			val_width = size * 2;
			is_signed = false;
			break;
		case std :: ios :: oct:
			val_width = ( ( size * 8 ) + 2 ) / 3;
			is_signed = false;
			break;
		default:
			switch ( size ) {
				case 1:
					val_width = 3;
					break;
				case 2:
					val_width = 5;
					break;
				default:
					val_width = 10;
					break;
			}
			if ( is_signed )
				val_width ++;
	}
	unsigned long mask = std :: numeric_limits < unsigned long > :: max ( );
	if ( size < sizeof ( mask ) )
		mask = ( 1L << ( size * 8 ) ) - 1;
	for ( unsigned i = 0; i < sSize; i += line_width ) {
		if ( i > 0 )
			os << '\n';
		for ( unsigned j = 0; j < indent; j ++ )
			os << ' ';
		for ( unsigned j = 0; j < line_width; j ++ ) {
			if ( j == line_width / 2 )
				os << ' ';
			if ( i + j < sSize ) {
				os << std :: setw ( val_width );
				if ( is_signed )
					os << int ( s [ i + j ] );
				else
					os << ( s [ i + j ] & mask );
			} else
				for ( unsigned k = 0; k < val_width; k ++ )
					os << ' ';
			os << ' ';
		}
		if ( ( os.flags ( ) & std :: ios :: floatfield ) != std :: ios :: fixed ) {
			os << "  ";
			for ( unsigned j = 0; j < line_width; j ++ ) {
				if ( i + j < sSize ) {
					if ( std :: isprint ( s [ i + j ] ) )
						os << char ( s [ i + j ] );
					else
						os << '.';
				}
			}
		}
	}
}

void printNumbers ( std :: ostream & os, const ss :: string & value, int indent ) {
	if ( value.size ( ) <= 32 || ( os.flags ( ) & std :: ios :: floatfield ) != std :: ios :: fixed ) {
		printNumbersOn ( os, value, value.size ( ) );
		os << '\n';
	} else {
		printNumbersOn ( os, value, 32 );
		os << '\n' << std :: setfill ( ' ' ) << std :: setw ( indent ) << "...\n";
	}
}
