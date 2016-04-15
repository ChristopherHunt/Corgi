test := push_pull_local_test
objs := push_pull_local_test.o

test_libs := lib_cache_api.a lib_logger.a lib_network.a lib_utils.a
target_dir := $(bin_test_dir)

run_test := cd $(bin_dir) &&\
			   mpirun -n 1 ./leader_layer $(bin_test_dir)/$(test)

include $(base_dir)/src/test.mk
