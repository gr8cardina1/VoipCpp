#include "rtpstat.hpp"
#include <cstring>
RTPStat :: RTPStat ( ) {
	std :: memset ( this, 0, sizeof ( * this ) );
}
