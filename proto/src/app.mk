include $(BASE_DIR)/src/common.mk

LDLIBS := $(addprefix $(BASE_DIR)/lib/, $(APP_LIBS))

$(BIN): $(LDLIBS) $(OBJS)
	$(CXX) $(LDFLAGS) $(LDLIBS) -o $@ $(OBJS)
