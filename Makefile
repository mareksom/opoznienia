BUILD = build
CPP = clang++
CPPFLAGS = -std=c++11 -g
LDFLAGS = -lboost_system -lpthread
OBJS = main.o Utils.o Options.o Error.o Data.o DataReader.o Dns.o Ping.o IOService.o UDPMethod.o ICMPMethod.o Ping.o MDns.o GetIPAddress.o IPList.o Measurement.o

all: $(BUILD)/run.e

$(BUILD)/run.e: $(addprefix $(BUILD)/, $(OBJS)) | $(BUILD)
	$(CPP) $^ -o $@ $(LDFLAGS)

$(BUILD)/%.o: %.cpp | $(BUILD)
	$(CPP) -c $^ -o $@ $(CPPFLAGS)

$(BUILD):
	mkdir -p $(BUILD)

.PHONY: clean

clean:
	rm -rf $(BUILD)
