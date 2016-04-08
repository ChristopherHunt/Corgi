include ../../common.mk

LDLIBS := $(addprefix ../../../lib/, $(APP_LIBS))

$(BIN): $(LDLIBS) $(OBJS)
	$(CXX) $(LDFLAGS) $(LDLIBS) -o $@ $(OBJS)
