#2006.6.12
#

TARGET_FILE = libToolClasses.so
COMPILEVER = $(shell date +"%Y%m%d")
CXXFLAGS = -Wall -DBUILD_DATE=$(COMPILEVER) -Dvfc=vd_vfc
CXX = g++
#CXX = arm-linux-g++
STRIP = strip
AR = 
LIBS = -lstdc++ -lpthread
INC = -I.


OBJECT =	./ComPort.o \
					./DataBuffer.o \
					./MutexInteger.o \
					./ProtoCollect.o \
					./TaskBase.o \
					./UdpSocket.o \
					./VorxFile.o \
					./VorxProto.o \
					./VorxThread.o \
					./VorxTime.o \
					./DES.o \
					./TcpClient.o \
					./TcpServer.o \
					./CyclePipe.o \
					
	
all: $(OBJECT) 
	$(CXX) -shared $(CXXFLAGS) $(OBJECT) -o $(TARGET_FILE) $(LIBS) $(INC)
	$(STRIP) $(OBJECT)

clean:
	rm -f *.o *.so

	

