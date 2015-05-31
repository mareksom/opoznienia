BUILD = build
CPP = g++
CPPFLAGS = -std=c++11 -g
LDFLAGS = -levent -lboost_system -lpthread
OBJS = main.o Options.o Error.o

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
