include $(BASE_DIR)/src/common.mk

$(LIB): $(OBJS)
	ar rcs $@ $^
