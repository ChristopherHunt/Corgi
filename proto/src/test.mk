include $(base_dir)/src/common.mk

ld_libs := $(addprefix $(base_dir)/lib/, $(test_libs))
gtest_dir := $(base_dir)/src/test/googletest
includes += -I$(gtest_dir) -I$(gtest_dir)/include -pthread

$(test): $(ld_libs) $(objs)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(ld_libs) -o $@ $(objs) $(includes)
	cp $@ $(bin_test_dir)
	./$@
	./$@ &> $@.log
	cp $@.log $(log_dir)
