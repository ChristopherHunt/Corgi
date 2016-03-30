#include <mpi.h>
#include <stdio.h>
#include "cache_node.h"

int main(int argc, char **argv) {
   MPI_Init(&argc, &argv);

#ifdef DEBUG
   printf("cache_node argc: %d\n", argc);
   for (int i = 0; i < argc; ++i) {
      printf("argv[%d]: %s\n", i, argv[i]);
   }
#endif

   std::string mapping(argv[1]);
   replace_commas(mapping);
   std::vector<uint32_t> map_vec;
   stringlist_to_vector(map_vec, mapping);

   CacheNode cache_node(map_vec);

   MPI_Finalize();

   return 0;
}
