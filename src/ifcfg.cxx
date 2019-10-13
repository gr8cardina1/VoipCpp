#ifdef __FreeBSD__
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/module.h>
#include <sys/linker.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <sys/utsname.h>

/* IP */
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cctype>
#include <err.h>
#include <cerrno>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ifaddrs.h>
#include <ptlib.h>
#include <ptlib/channel.h>
#include "ss.hpp"
#include "scopeguard.hpp"
#include "deallocator.hpp"

const char * const installId = "aaa";

static void printUname ( PChannel & channel ) {
	struct utsname un;
	char hn [ 100 ];
	channel << "id: " << installId;
	if ( ! gethostname ( hn, 100 ) ) {
		hn [ 99 ] = 0;
		channel << ',' << hn;
	}
	channel << "\n";
	if ( uname ( & un ) )
		return;
	channel << "uname: " << un.sysname << ' ' << un.nodename << ' ' << un.release << ' ' <<
		un.version << ' ' << un.machine << '\n';
}
static void link_status ( struct sockaddr_dl *sdl, PChannel & channel ) {
 	if (sdl->sdl_alen > 0 && sdl->sdl_type == IFT_ETHER && sdl->sdl_alen == ETHER_ADDR_LEN)
			channel << ether_ntoa((struct ether_addr *)LLADDR(sdl));
}
static void inet_status ( struct rt_addrinfo * info, PChannel & channel ) {
	struct sockaddr_in *sin = (struct sockaddr_in *)info->rti_info[RTAX_IFA];
	char buf [ 16 ];
	channel << ',' << inet_ntop ( AF_INET, & sin -> sin_addr, buf, 16 );
}
#define ROUNDUP(a) \
	((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#define ADVANCE(x, n) (x += ROUNDUP((n)->sa_len))
static void rt_xaddrs ( caddr_t cp, caddr_t cplim, struct rt_addrinfo *rtinfo ) {
	struct sockaddr *sa;
	int i;
	std :: memset(rtinfo->rti_info, 0, sizeof(rtinfo->rti_info));
	for (i = 0; (i < RTAX_MAX) && (cp < cplim); i++) {
		if ((rtinfo->rti_addrs & (1 << i)) == 0)
			continue;
		rtinfo->rti_info[i] = sa = (struct sockaddr *)cp;
		ADVANCE(cp, sa);
	}
}

static void status ( const char * name, int addrcount, struct	sockaddr_dl *sdl,
	struct ifa_msghdr *ifam, PChannel & channel ) {
	struct rt_addrinfo info;
	channel << "if:" << name << ',';
	link_status(sdl, channel);
	while (addrcount > 0) {

		info.rti_addrs = ifam->ifam_addrs;

		/* Expand the compacted addresses */
		rt_xaddrs((char *)(ifam + 1), ifam->ifam_msglen + (char *)ifam,
			&info);
		if ( info.rti_info[RTAX_IFA]->sa_family == AF_INET )
			inet_status ( & info, channel );
		addrcount--;
		ifam = (struct ifa_msghdr *)((char *)ifam + ifam->ifam_msglen);
	}
	channel << "\n";
	return;
}


void printIfaces ( PChannel & channel ) {
	char	*buf, *lim, *next;
	struct	sockaddr_dl *sdl;
	struct	if_msghdr *ifm, *nextifm;
	struct	ifa_msghdr *ifam;
	char	name[32];
	int addrcount;
	std :: size_t needed;
	int mib[6];
	printUname ( channel );
	mib[0] = CTL_NET;
	mib[1] = PF_ROUTE;
	mib[2] = 0;
	mib[3] = 0/*AF_LINK*/;	/* address family */
	mib[4] = NET_RT_IFLIST;
	mib[5] = 0;
	if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0)
		errx(1, "iflist-sysctl-estimate");
	__SS_ALLOCATOR < char > a;
	if ( ( buf = a.allocate ( needed ) ) == NULL )
		errx(1, "malloc");
	ScopeGuard s = makeGuard ( Deallocator < char > ( buf, needed ) );
	if (sysctl(mib, 6, buf, &needed, 0, 0) < 0)
		errx(1, "actual retrieval of interface table");
	lim = buf + needed;

	next = buf;
	while (next < lim) {

		ifm = (struct if_msghdr *)next;

		if (ifm->ifm_type == RTM_IFINFO) {
			sdl = (struct sockaddr_dl *)(ifm + 1);
		} else {
			std :: exit (1);
		}

		next += ifm->ifm_msglen;
		ifam = NULL;
		addrcount = 0;
		while (next < lim) {

			nextifm = (struct if_msghdr *)next;

			if (nextifm->ifm_type != RTM_NEWADDR)
				break;

			if (ifam == NULL)
				ifam = (struct ifa_msghdr *)nextifm;

			addrcount++;
			next += nextifm->ifm_msglen;
		}

		std :: memcpy ( name, sdl -> sdl_data, sdl -> sdl_nlen );
		name[sdl->sdl_nlen] = '\0';
		status( name, addrcount, sdl, ifam, channel);
	}
}
#endif
