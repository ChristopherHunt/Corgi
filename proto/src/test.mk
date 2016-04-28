include $(base_dir)/src/common.mk

ld_libs := $(addprefix $(base_dir)/lib/, $(test_libs))
gtest_dir := $(base_dir)/src/test/googletest
#includes += -I$(gtest_dir) -I$(gtest_dir)/include -pthread
includes += -I$(gtest_dir) -I$(gtest_dir)/include
#temp := ""

$(test): $(ld_libs) $(objs)
	$(MPICXX) $(CXXFLAGS) $(LDFLAGS) $(objs) $(ld_libs) -o $@ $(includes)
	cp $@ $(bin_test_dir)
	#./$@
	$(run_test) &> $(log_dir)/$@.log
