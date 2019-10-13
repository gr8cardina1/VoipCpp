#pragma implementation
#include <ptlib.h>
#include "automutex.hpp"
AutoMutex :: AutoMutex ( PMutex * mm ) : m ( mm ) {
	m -> Wait ( );
}
AutoMutex :: AutoMutex ( PMutex & mm ) : m ( & mm ) {
	m -> Wait ( );
}
AutoMutex :: ~AutoMutex ( ) {
	m -> Signal ( );
}
AntiMutex :: AntiMutex ( PMutex * mm ) : m ( mm ) {
	m -> Signal ( );
}
AntiMutex :: AntiMutex ( PMutex & mm ) : m ( & mm ) {
	m -> Signal ( );
}
AntiMutex :: ~AntiMutex ( ) {
	m -> Wait ( );
}
