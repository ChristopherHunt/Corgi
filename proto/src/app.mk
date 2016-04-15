include $(base_dir)/src/common.mk

ld_libs := $(addprefix $(base_dir)/lib/, $(app_libs))

$(app): $(ld_libs) $(objs)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(ld_libs) -o $@ $(objs)
	cp $@ $(target_dir)
