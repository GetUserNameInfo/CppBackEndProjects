CC=gcc
CXX=g++
RM=rm -f
FLAGS=-g
LIBS=

SRCS=Utility.cc FileOpt.cc Acceptor.cc Socket.cc FtpServer.cc 
OBJS=$(subst .cc,.o,$(SRCS))

all: ftpserver

ftpserver: $(OBJS)
	$(CXX) $(FLAGS) -o ftpserver $(OBJS) $(LIBS)
	$(RM) $(OBJS) 

clean: 
	$(RM) ftpserver
