#
# Makefile
# 
# Make file for OpenH323 Gatekeeper
#
# Copyright (c) Egoboo Ltd. 1999-2000
#
# The contents of this file are subject to the Mozilla Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
# the License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is Open Gatekeeper
#
# The Initial Developer of the Original Code is Egoboo Ltd.
#
# $Log: Makefile,v $
# Revision 1.50  2006/03/28 15:15:15  alm
# IxcUDPSocket class added
# 
# Revision 1.49  2005/10/07 12:45:50  pal
# before cards cache
#
# Revision 1.48  2005/08/03 08:12:31  pal
# before sip reinvite
#
# Revision 1.47  2005/04/19 08:54:32  pal
# before fixes
#
# Revision 1.46  2005/04/11 15:01:00  pal
# after sip registration
#
# Revision 1.45  2005/03/30 11:45:07  pal
# SIP :: MIMEInfo moved to boost
#
# Revision 1.44  2005/03/11 13:16:10  pal
# before sip->h323
#
# Revision 1.43  2005/03/03 16:19:44  pal
# fixed tarball
#
# Revision 1.42  2005/02/21 09:58:21  pal
# before boost
#
# Revision 1.41  2005/01/27 00:27:57  pal
# before allocatable
#
# Revision 1.40  2004/12/14 13:13:50  pal
# before findmapelement change
#
# Revision 1.39  2004/12/07 14:43:57  pal
# slow start broken, before fixing
#
# Revision 1.38  2004/11/08 15:11:28  pal
# before iterations split
#
# Revision 1.37  2004/09/29 11:52:37  pal
# before sip read from buf
#
# Revision 1.36  2004/08/10 11:06:53  pal
# big changes
#
# Revision 1.35  2004/03/29 01:06:30  pal
# cardeaters
#
# Revision 1.34  2004/03/16 14:33:29  pal
# before price number lengths
#
# Revision 1.33  2004/03/04 15:17:10  pal
# partially working calls to cards
#
# Revision 1.32  2004/01/22 15:57:22  pal
# before out ports
#
# Revision 1.31  2004/01/07 07:50:26  pal
# temp
#
# Revision 1.30  2003/12/02 15:59:50  pal
# first runnable radius
#
# Revision 1.29  2003/10/16 14:22:19  pal
# before condvar use
#
# Revision 1.28  2003/09/17 09:03:38  pal
# before break
#
# Revision 1.27  2003/08/27 17:22:10  pal
# tarif round added
#
# Revision 1.26  2003/06/25 09:15:10  pal
# opal
#
# Revision 1.25  2003/05/07 14:25:45  pal
# fix for master prefixes
#
# Revision 1.24  2003/04/29 08:40:48  pal
# fixes
#
# Revision 1.23  2003/04/15 14:55:51  pal
# before gcc3
#
# Revision 1.22  2003/04/04 18:27:56  pal
# registration rewritten in pwlib
#
# Revision 1.21  2003/03/31 16:17:59  pal
# tipa stable
#
# Revision 1.20  2003/03/13 10:22:28  pal
# before break
#
# Revision 1.19  2003/02/18 10:07:38  pal
# before huntstop
#
# Revision 1.18  2002/12/19 21:44:01  pal
# probably fixed gr bug.
#
# Revision 1.17  2002/12/19 21:40:41  pal
# probably fixed gr bug.
#
# Revision 1.16  2002/12/17 18:53:41  pal
# before conf change.
#
# Revision 1.15  2002/11/19 14:26:00  pal
# fix for dialogik.
#
# Revision 1.14  2002/11/14 06:31:20  pal
# random fixes.
#
# Revision 1.13  2002/10/31 17:26:00  pal
# added tuneling.
#
# Revision 1.12  2002/06/12 09:21:34  pal
# workaround for prodictId=VoIP.
#
# Revision 1.11  2002/03/27 16:11:49  pal
# regclient & dropTokens & 1 write
#
# Revision 1.10  2002/03/20 12:27:58  pal
# rtp stats.
#
# Revision 1.9  2002/03/15 14:03:44  pal
# tons of fixes. fs should work.
#
# Revision 1.8  2002/03/02 13:27:06  pal
# h245handler added.
#
# Revision 1.7  2002/02/11 14:50:21  pal
# rtp slave compiled cleanly.
#
# Revision 1.6  2002/02/08 21:14:04  pal
# before separate process.
#
# Revision 1.5  2002/02/05 11:00:26  pal
# first try at one rtp thread.
#
# Revision 1.4  2002/01/19 22:21:32  pal
# added support for updateId.
#
# Revision 1.3  2002/01/18 01:36:57  pal
# various fixes.
#
# Revision 1.2  2001/12/01 02:19:41  pal
# fixed build at home.
#
# Revision 1.1.1.1  2001/12/01 02:09:40  pal
# Imported sources
#
# Revision 1.15  2000/05/12 14:06:30  aunitt
# Renamed PRasLog to Log
#
# Revision 1.14  2000/05/03 13:10:10  aunitt
# Removed bogus include
#
# Revision 1.13  2000/05/02 16:31:09  aunitt
# Semi automated tarball build.
#
# Revision 1.12  2000/04/27 17:59:05  aunitt
# Updated tarball target.
# (Really do need to sort this out!)
#
# Revision 1.11  2000/04/21 13:41:41  aunitt
# Updated to version 0.6
#
# Revision 1.10  2000/04/20 18:48:38  aunitt
# Added Environ.cxx
#
# Revision 1.9  2000/04/10 19:18:35  aunitt
# Added support for Gatekeeper Routed Calls.
#
# Revision 1.8  2000/04/05 15:20:01  aunitt
# Added PRasLog object.
#
# Revision 1.7  2000/03/23 00:03:19  aunitt
# Upped build number
#
# Revision 1.6  2000/03/21 21:00:58  aunitt
# Fixed working with latest OpenH323 code.
# Patch thanks to Craig Southern.
#
# Revision 1.5  2000/03/19 17:37:11  aunitt
# Should now compile in any directory.
#
# Revision 1.4  2000/02/27 20:47:19  aunitt
# Added windows support. Fixed reply to wrong RAS address
#
# Revision 1.3  2000/02/09 22:31:09  aunitt
# Fixed not putting the Makefile in the tarball
#
# Revision 1.2  2000/02/06 19:16:09  aunitt
# Added tarball target
#
# Revision 1.1.1.1  2000/02/01 22:25:34  aunitt
# Initial revision
#
#
LDFLAGS += -L/usr/local/lib/gcc-4.3.1 -L/usr/local/lib/mysql -L/usr/lib64/mysql
DEBLDFLAGS =
STDCCFLAGS += -I. -Wextra -Wfloat-equal -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wmissing-format-attribute -Wpacked -Wredundant-decls -Winvalid-pch -Woverloaded-virtual
ifeq ($(WITH_FREPO),yes)
STDCCFLAGS += -frepo
endif
CFLAGS += -I.
# -Werror
#mysql.cxx_CXXFLAGS=-Wno-ignored-qualifiers
outpriceelement.cxx_CXXFLAGS=-Wno-conversion
#backtrace.cxx_CXXFLAGS=-Wno-ignored-qualifiers
#sipsignallingthread.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#signallingthread.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#sip.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#h323common.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#h323leg.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#sipleg.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#mgcpleg.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#h323answerleg.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#sipanswerleg.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#mgcpanswerleg.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#answercall.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#originatelegthread.cxx_CXXFLAGS=-Wsign-compare -Wno-extra -Wmissing-field-initializers
#New.cxx_CXXFLAGS=-Wno-old-style-cast
#mgcpthread.cxx_CXXFLAGS=-Wno-unsafe-loop-optimizations
#moneyspool.cxx_CXXFLAGS=-Wno-float-equal
sipgrammars.cxx_CXXFLAGS=-O3
sdpgrammars.cxx_CXXFLAGS=-O3
PROG		= psbc_core
SOURCES		= New.cxx mysql.cxx dbconf.cxx AddrUtils.cxx Log.cxx \
		  automutex.cxx priceelement.cxx getindex.cxx outchoice.cxx \
		  outpriceelement.cxx Conf.cxx sqloutthread.cxx CallThread.cxx signallingthread.cxx \
                  h245thread.cxx confthread.cxx rtppipe.cxx h245handler.cxx \
                  rtpsession.cxx rtpstat.cxx rasthread.cxx confdata.cxx md5.cxx endthread.cxx \
                  SipCallThread.cxx sipsignallingthread.cxx tarifround.cxx \
                  condvar.cxx radius.cxx radiusthread.cxx outchoicedetails.cxx outcarddetails.cxx \
                  h323.cxx tarifinfo.cxx moneyspool.cxx customercalls.cxx \
                  geganet.cxx sip.cxx mgcpthread.cxx codecinfo.cxx \
                  mgcp.cxx pricedata.cxx basegkclientthread.cxx h323gkclientthread.cxx \
                  sipgkclientthread.cxx h323call.cxx h323common.cxx \
                  h323leg.cxx sipleg.cxx legthread.cxx h323answerleg.cxx sipcommon.cxx answercall.cxx \
                  answerlegthread.cxx fakecalldetector.cxx sipanswerleg.cxx SIPAuthenticateValidator.cxx \
                  profitguard.cxx callbuffer.cxx RegisteredOutPeersMap.cxx \
                  httpthread.cxx activeresource.cxx ixcudpsocket.cxx totalactiveresource.cxx debugstream.cxx \
                  httplistenthread.cxx priceprios.cxx aftertask.cxx radiuscommon.cxx h225.cxx h235.cxx h245.cxx t38.cxx \
                  backtrace.cxx mgcpanswerleg.cxx mgcpepstatehandler.cxx mgcpep.cxx mgcpepcallhandler.cxx \
                  mgcpepnotifyhandler.cxx mgcpepstarthandler.cxx mgcpgw.cxx ivractiveresource.cxx originatelegthread.cxx \
                  mgcpleg.cxx mgcpeporiginatehandler.cxx mgcpsignalparser.cxx commoncalldetails.cxx sdp.cxx sdpcommon.cxx \
                  mgcpepdlcxhandler.cxx q931.cxx printnumbers.cxx callpathresource.cxx sip2.cxx siptransportthread.cxx \
                  sipleg2.cxx sipanswerleg2.cxx random.cxx sip2gkclientthread.cxx sipgrammars.cxx ntptimestamp.cxx \
                  sdpgrammars.cxx dtmfgrammars.cxx dtmfrelay.cxx ipinpeers.cxx translateipmask.cxx

#Proxy.cxx EndpointTabl.cxx RasThread.cxx RasServer.cxx MulticastThread.cxx CallTabl.cxx CallThread.cxx Environ.cxx ProxyThread.cxx ProxyCriteria.cxx RouteTbl.cxx opengate.cxx rtppchannel.cxx ProxyTabl.cxx
TAR_SOURCES	:= $(SOURCES) asnparser/asn_grammar.y asnparser/asn_lex.l asnparser/main.h asnparser/main.cxx \
		  asnparser/Makefile Makefile AddrUtils.h Log.h CallThread.h Version.h \
		  tarball.mak setver/Makefile setver/setver.cxx mysql.cpp dbconf.cpp Conf.hpp \
		  calldetails.hpp automutex.hpp New.h dbconf.hpp priceelement.hpp \
		  outpriceelement.hpp mysql.hpp pointer.hpp unknowndigit.hpp outchoice.hpp \
		  sqloutthread.hpp confthread.hpp rtppipe.hpp rasthread.hpp popt.h \
		  getindex.hpp signallingthread.hpp rtpstat.hpp h245handler.hpp h245thread.hpp \
		  rtprequest.hpp rtpsession.hpp ifcfg.cxx confdata.hpp sourcedata.hpp md5.hpp endthread.hpp \
		  ssconf.hpp SipCallThread.h sipsignallingthread.hpp tarifround.hpp \
		  condvar.hpp radius.hpp requestinfo.hpp radiusthread.hpp signallingoptions.hpp \
		  customercalls.hpp outchoicedetails.hpp findprefix.hpp registeredcard.hpp \
		  outcarddetails.hpp h323.hpp asn.hpp q931.hpp h225.hpp h235.hpp h235_t.cxx h245.hpp t38.hpp \
		  tarifinfo.hpp moneyspool.hpp geganet.hpp mgcpthread.hpp radgwinfo.hpp codecinfo.hpp \
		  session.hpp mgcp.hpp pricedata.hpp \
		  basegkclientthread.hpp h323gkclientthread.hpp sipgkclientthread.hpp firstiterator.hpp \
		  setthreeway.hpp h323call.hpp h323common.hpp h323leg.hpp \
		  sipleg.hpp legthread.hpp callcontrol.hpp findmapelement.hpp h323answerleg.hpp sipcommon.hpp \
		  allocatable.hpp ss.hpp sip.hpp ipport.hpp icqcontact.hpp sipcalldetails.hpp \
		  accountinginfo.hpp rtpslave/Makefile rtpslave/rtpprocess.cxx rtpslave/rtpprocess.hpp \
		  rtpslavebsd/Makefile rtpslavebsd/ip_ss.h rtpslavebsd/rtpprocess.cxx rtpslavebsd/rtpprocess.hpp \
		  rtpslave/rtpstat.cxx answercall.hpp answerlegthread.hpp fakecalldetector.hpp latencylimits.hpp \
		  sipanswerleg.hpp SIPAuthenticateValidator.hpp profitguard.hpp callbuffer.hpp \
		  RegisteredOutPeersMap.hpp commoncalldetails.hpp serializestring.hpp httpthread.hpp activeresource.hpp \
		  ixcudpsocket.hpp totalactiveresource.hpp debugstream.hpp httplistenthread.hpp \
		  priceprios.hpp aftertask.hpp radiuscommon.hpp printhex.hpp backtrace.hpp istringless.hpp \
		  mgcpanswerleg.hpp threadmessagequeue.hpp mgcpepstatehandler.hpp mgcpep.hpp mgcpepcallhandler.hpp \
		  mgcpepnotifyhandler.hpp mgcpepstarthandler.hpp mgcpgw.hpp mgcpepthreadhandler.hpp rightparser.hpp \
		  ivractiveresource.hpp originatelegthread.hpp slprint.hpp scopeguard.hpp mgcpleg.hpp \
		  mgcpeporiginatehandler.hpp mgcpsignalparser.hpp mgcporiginatecallarg.hpp deallocator.hpp smartguard.hpp \
		  sdp.hpp namespair.hpp sdpcommon.hpp mgcpepdlcxhandler.hpp printnumbers.hpp nop.hpp transformiterator.hpp \
		  mgcpconf.hpp callpathresource.hpp prototype.hpp sip2.hpp hostnamegrammars.hpp sipleg2.hpp \
		  siptransportthread.hpp siporiginatehandler.hpp sipanswerhandler.hpp sipanswerleg2.hpp random.hpp sip2gkclientthread.hpp \
		  sipgrammars.hpp ntptimestamp.hpp jmake nonaliasable.hpp sdpgrammars.hpp dtmfgrammars.hpp dtmfrelay.hpp ipinpeers.hpp \
		  translateipmask.hpp

export TAR_SOURCES

ifndef PWLIBDIR
PWLIBDIR:=$(shell pwd)/../pwlib
endif

ifndef DEBUG_FLAG
#DEBUG_FLAG      := -g2
#DEBUG_FLAG     += -gstabs+
DEBUG_FLAG	:= -g3 -gdwarf-2
DEBUG_FLAG	+= -O0
#STDCCFLAGS +=  -Werror -D_GLIBCXX_CONCEPT_CHECKS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
#STDCCFLAGS +=  -Wall -Werror -D_GLIBCXX_CONCEPT_CHECKS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
STDCCFLAGS +=  -Wall
# -Werror
endif

ifeq ($(wildcard /usr/include/stdint.h),)
STDCCFLAGS += -I./fbsd4
endif

OPTCCFLAGS += -g3

include $(PWLIBDIR)/make/unix.mak

ifeq ($(OSTYPE),FreeBSD)
SOURCES += ifcfg.cxx
LDLIBS += /usr/local/lib/mysql/libmysqlclient_r.a -lz
STDCCFLAGS += -isystem $(IXC_BOOST_INCLUDE)
else
LDLIBS += -lmysqlclient_r
CPUTYPE =athlon-xp -march=athlon-xp
STDCCFLAGS += -Wundef -isystem /home/pal/tmp/boost/include/boost-1_38
endif


include $(PWLIBDIR)/make/common.mak

ifdef DEBUG
ifndef RDYNAMIC
RDYNAMIC = -rdynamic
endif
endif

ifeq ($(OSTYPE),FreeBSD)
  ifdef DEBUG
    LDLIBS += $(IXC_BOOST_LIB)/libboost_serialization-mt-d.a $(IXC_BOOST_LIB)/libboost_regex-mt-d.a $(IXC_BOOST_LIB)/libboost_date_time-mt-d.a
  else
    LDLIBS += $(IXC_BOOST_LIB)/libboost_serialization-gcc42-mt.a $(IXC_BOOST_LIB)/libboost_regex-gcc42-mt.a $(IXC_BOOST_LIB)/libboost_date_time-gcc42-mt.a
  endif
else
  LDFLAGS += -Wl,-rpath,$(PWLIBDIR)/lib -L/opt/psbc/src/boost/stage/lib
  ifeq ($(wildcard /usr/lib/libboost_serialization-gcc-mt.so),)
    ifdef DEBUG
      LDLIBS += -lboost_serialization-mt-d -lboost_regex-mt-d -lboost_date_time-mt-d
    else
      LDLIBS += -lboost_serialization-mt -lboost_regex-mt -lboost_date_time-mt
    endif
  else
    ifdef DEBUG
      LDLIBS += -lboost_serialization-gcc-mt-d -lboost_regex-gcc-mt-d -lboost_date_time-gcc-mt-d
    else
      LDLIBS += -lboost_serialization-gcc-mt -lboost_regex-gcc-mt -lboost_date_time-gcc-mt
    endif
  endif
endif

LDFLAGS += $(RDYNAMIC)

tarball:
	$(MAKE) -C setver debug
	./setver/obj_$(PLATFORM_TYPE)_d/setver Version.inc
	$(MAKE) -f tarball.mak

