# Source, Includes
recommendation RECOMMENDATION
	INCL_RECOMMENDATION = lib/in_out/arg_parser.h ./lib/in_out/vector_reader.hpp ./lib/data_structures/cust_vector.hpp ./lib/data_structures/cust_hashtable.hpp ./lib/data_structures/vector_bucket.hpp ./lib/utils.hpp ./lib/generators/euclidean_h_gen.hpp ./lib/generators/euclidean_phi_gen.hpp ./lib/generators/cosine_h_gen.hpp ./lib/generators/cosine_g_gen.hpp ./lib/generators/hash_generator.hpp ./lib/generators/euclidean_f_gen.hpp ./lib/generators/hypercube_gen.hpp ./lib/clustering_phases/initialization.hpp ./lib/clustering_phases/assignment.hpp ./lib/clustering_phases/silhouette.hpp ./lib/clustering_phases/update.hpp ./lib/lsh_cube.hpp ./lib/data_structures/tweet.h ./lib/crypto_rec.hpp

    SRC_RECOMMENDATION = main.cpp ./lib/in_out/arg_parser.cpp ./lib/utils.cpp ./lib/data_structures/tweet.cpp

	OBJ_RECOMMENDATION = $(SRC_CLUSTER:.cpp=.o)

	PROG_RECOMMENDATION = recommendation

# Compiler, Linker Defines
	CC      = g++ -g
	RM      = rm -f

# Compile and Assemble C++ Source Files into Object Files
%.o: $(SRC_RECOMMENDATION)/%.cpp
	$(CC) -c $*.cpp

$(PROG_RECOMMENDATION): $(OBJ_RECOMMENDATION)
	$(CC) -o $(PROG_RECOMMENDATION) $(OBJ_RECOMMENDATION)

$(OBJ_RECOMMENDATION): $(INCL_RECOMMENDATION)

# Clean Up Exectuables
clean:
	$(RM) $(PROG_RECOMMENDATION) $(OBJ_RECOMMENDATION)