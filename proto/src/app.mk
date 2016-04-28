include $(base_dir)/src/common.mk

ld_libs := $(addprefix $(base_dir)/lib/, $(app_libs))

$(app): $(ld_libs) $(objs)
	$(MPICXX) $(CXXFLAGS) $(objs) $(LDFLAGS) $(ld_libs) -o $@
	cp $@ $(target_dir)
