CPP	:= g++
CC	:= gcc
SRCS	:= As.cpp Link.cpp Network.cpp Node.cpp Router.cpp SamplingPlatform.cpp TraficLoader.cpp TransitNetwork.cpp tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp tinystr.cpp exception.cpp SoftflowdInterface.cpp TimeStamp.cpp SamplingManager.cpp
OBJS	:=$(addsuffix .o,$(basename ${SRCS})) 
LIBS= -lpthread -lpcap
MAIN	:= main.cpp
OPT	:= -O3 -fomit-frame-pointer -s

all : SPlatform 

SPlatform : $(OBJS) $(MAIN)
	$(CPP) $(MAIN) $(OBJS) -o $@ $(LIBS)
%.o : %.cpp
	${CXX} -c -g  ${CXXFLAGS} ${INCS} $< -o $@ 

%.o : %.c
	${CC} -c -g ${CFLAGS} ${INCS} $< -o $@ 

Link.o: Link.cpp Link.h CommonStructs.h
Node.o: Node.cpp Node.h CommonStructs.h
Router.o: Router.cpp Router.h Node.h Node.cpp CommonStructs.h
Network.o: Network.cpp Network.h CommonStructs.h
TransitNetwork.o: TransitNetwork.cpp TransitNetwork.h Network.h Network.cpp As.h As.cpp
As.o: As.cpp As.h CommonStructs.h
TraficLoader.o: TraficLoader.cpp TraficLoader.h CommonStructs.h
SamplingPlatform.o: SamplingPlatform.cpp SamplingPlatform.h TraficLoader.h TraficLoader.cpp TransitNetwork.h TransitNetwork.cpp TimeStamp.cpp CommonStructs.h
tinyxml.o: tinyxml.h tinystr.h CommonStructs.h
tinyxmlparser.o: tinyxml.h tinystr.h CommonStructs.h
xmltest.o: tinyxml.h tinystr.h CommonStructs.h
tinyxmlerror.o: tinyxml.h tinystr.h CommonStructs.h
exception.o: exception.h CommonStructs.h
SoftflowdInterface.o:  SoftflowdInterface.h CommonStructs.h
TimeStamp.o: TimeStamp.h CommonStructs.h
SamplingManager.o: SamplingManager.h

clean :
	-rm *.o *~ core paper
	-clear
