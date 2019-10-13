#ifndef __RTPSTAT_HPP
#define __RTPSTAT_HPP
#pragma interface

struct RTPStat : public Allocatable < __SS_ALLOCATOR > {
	unsigned count;
	unsigned fracLostSum;
	unsigned packLost;
	unsigned jitterMax;
	unsigned jitterSum;
	unsigned firstSequence;
	unsigned lastSequence;
	unsigned packetCount;
	unsigned bytesCount;
	RTPStat ( );
};
#endif
