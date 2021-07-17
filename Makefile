SRC = src/main.cpp src/bridge.cpp
HDR = src/bridge.h src/wsgameserver.h
CFLAGS = -Wall -Os -s -ffunction-sections -fdata-sections -Wl,--gc-sections -Isubprojects/json -Isubprojects/valijson/include

UATSERVERPP_HDR = subprojects/UATServerPP/include/uatserverpp.h
UATSERVERPP_LIB = # header-only
UATSERVERPP_DEP = $(UATSERVER_HDR) 
UATSERVERPP_CFLAGS = -pthread -Isubprojects/UATServerPP/include -Isubprojects/asio/include -Isubprojects/json/include -Isubprojects/valijson/include -Isubprojects

WIN_LIBS = -lwsock32 -lws2_32
WIN_CFLAGS = -static

uatbridge: $(SRC) $(HDR) $(UATSERVERPP_DEP)
	$(CXX) -o $@ $(CFLAGS) $(SRC) $(UATSERVERPP_CFLAGS) $(UATSERVERPP_LIB)


uatbridge.exe: $(SRC) $(HDR) $(UATSERVERPP_DEP)
	$(CXX) -o $@ $(CFLAGS) $(WIN_CFLAGS) $(SRC) $(UATSERVERPP_CFLAGS) $(UATSERVERPP_LIB) $(WIN_LIBS)

cross: uatbridge.exe

clean:
	$(RM) uatbridge
	$(RM) uatbridge.exe
