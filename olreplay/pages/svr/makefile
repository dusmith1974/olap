CXXFLAGS=-Wall -Wno-sign-compare -std=c++11
INC=-I$(POCO_BASE)/CppUnit/include
INC+=-I$(POCO_BASE)/Foundation/include
INC+=-I$(POCO_BASE)/XML/include
INC+=-I$(POCO_BASE)/Util/include
INC+=-I$(POCO_BASE)/Net/include
DEF=-DPOCO_BUILD_HOST=x201
DEF+=-D_XOPEN_SOURCE=500
DEF+=-D_REENTRANT
DEF+=-D_THREAD_SAFE
DEF+=-D_FILE_OFFSET_BITS=64
DEF+=-D_LARGEFILE64_SOURCE
DEF+=-DPOCO_HAVE_FD_EPOLL
DEF+=-g

all:
	mkdir -p obj
	mkdir -p bin
	g++ $(INC) $(CXXFLAGS) $(DEF) -D_DEBUG -fPIC -c src/svr.cpp -o obj/svr.o
	g++ $(DEF) -Wl,-rpath,lib -o bin/svrd obj/svr.o -L$(POCO_BASE)/lib/Linux/x86_64 -lPocoUtild -l PocoNetd -lPocoXMLd -lPocoFoundationd -lpthread -ldl -lrt

clean:
	rm bin obj -rf

