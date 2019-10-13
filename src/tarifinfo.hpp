#ifndef __TARIFINFO_HPP
#define __TARIFINFO_HPP
#pragma interface

struct TarifInfo : public Allocatable < __SS_ALLOCATOR > {
	IntIntMap minsToSecs;
	int getSeconds ( double mins ) const;
	double getMinutes ( int seconds ) const;
};
#endif
