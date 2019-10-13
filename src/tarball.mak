include Version.inc

all:
	(cd ..; tar jcf ss_$(GKVER).tar.bz2 $(patsubst %, $(notdir $(shell pwd))/%, $(TAR_SOURCES)) )	


		
