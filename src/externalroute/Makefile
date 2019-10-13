CXXFLAGS=-g3 -I..
CPLUS = g++

externalroute: externalroute.o radius.o md5.o
	$(CPLUS) $(CXXFLAGS) -o $@ $^

externalroute.o: externalroute.cpp ../radius.hpp
	$(CPLUS) -o $@ -c $(CXXFLAGS) $<

radius.o: ../radius.cxx ../radius.hpp
	$(CPLUS) -o $@ -c $(CXXFLAGS) $<

md5.o: ../md5.c
	gcc -o $@ -c $(CXXFLAGS) $<


clean:
	rm -f externalroute.o radius.o md5.o externalroute
