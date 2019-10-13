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
# Revision 1.2  2002/12/17 18:53:41  pal
# before conf change.
#
# Revision 1.1.1.1  2001/12/01 02:09:40  pal
# Imported sources
#
# Revision 1.1  2000/05/02 16:38:08  aunitt
# Makefile for setver
#
#
#

PROG		= setver
SOURCES		= setver.cxx

#ifndef PWLIBDIR
PWLIBDIR=$(PWD)/../pwlib
#endif

ifndef OPENGATEDIR
OPENGATEDIR=..
endif

include $(PWLIBDIR)/make/ptlib.mak

STDCCFLAGS += -I$(OPENGATEDIR)

