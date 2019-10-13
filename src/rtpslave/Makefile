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
# Revision 1.3  2002/03/20 12:27:58  pal
# rtp stats.
#
# Revision 1.2  2002/03/15 14:03:44  pal
# tons of fixes. fs should work.
#
# Revision 1.1  2002/02/11 14:50:22  pal
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
DEBLDFLAGS = -static

PROG		= rtpslave
SOURCES		= rtpslave.cxx

rtpslave: rtpprocess.o
	g++ -static -s -O2 -o rtpslave rtpprocess.o ../ob*d/rtpstat.o

rtpprocess.o: rtpprocess.cxx rtpprocess.hpp
	g++ -o rtpprocess.o -c -O2 -I.. rtpprocess.cxx

