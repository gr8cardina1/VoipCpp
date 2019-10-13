#pragma implementation
#include <ptlib.h>
#include <ptlib/sockets.h>
#include "ss.hpp"
#include "allocatable.hpp"
#include "basegkclientthread.hpp"

BaseGkClientThread :: BaseGkClientThread ( ) : PThread ( 1000, NoAutoDeleteThread, NormalPriority ) { }



