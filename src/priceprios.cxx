#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include "signallingoptions.hpp"
#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "outcarddetails.hpp"
#include "outchoice.hpp"
#include "priceprios.hpp"

PricePrios :: PricePrios ( ) : f1 ( & OutChoice :: getMinusDepth ), f2 ( & OutChoice :: getPrio ),
	f3 ( & OutChoice :: getPrice ) { }
