#ifndef __RADGWINFO_HPP
#define __RADGWINFO_HPP
#pragma interface

struct RadGWInfo {
	ss :: string secret;
	bool useAni;
	bool useDnis;
	bool useAcode;
	bool verifySecret;
	int defaultServiceType;
	bool hasStart;
	bool sendDuration;
	bool unameOnly;
};
#endif
