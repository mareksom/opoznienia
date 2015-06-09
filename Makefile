BUILD = build
CPP = g++
CPPFLAGS = -std=c++11 -O2
LDFLAGS = -lboost_system -lpthread
OBJS = opoznienia.o Utils.o Options.o Error.o \
	IOService.o \
	GetIPAddress.o IPList.o \
	Data.o DataReader.o IPv4Header.o \
	Dns.o Ping.o MDns.o \
	UDPMethod.o TCPMethod.o ICMPMethod.o Timeout.o \
	Measurement.o \
	UIServer.o

all: $(BUILD)/opoznienia

$(BUILD)/opoznienia: $(addprefix $(BUILD)/, $(OBJS)) | $(BUILD)
	$(CPP) $^ -o $@ $(LDFLAGS)

$(BUILD)/%.o: %.cpp | $(BUILD)
	$(CPP) -c $^ -o $@ $(CPPFLAGS)

$(BUILD):
	mkdir -p $(BUILD)

.PHONY: clean

clean:
	rm -rf $(BUILD)
