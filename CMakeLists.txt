cmake_minimum_required(VERSION 3.10)
project(Cluster)

set(CMAKE_CXX_STANDARD 11)

include_directories(lib lib/data_structures lib/generators lib/in_out lib/clustering_phases)

add_executable(cluster
        main.cpp
        lib/in_out/arg_parser.cpp
        lib/in_out/arg_parser.h
        lib/generators/hash_generator.hpp
        lib/data_structures/cust_vector.hpp
        lib/in_out/vector_reader.hpp
        lib/utils.cpp
        lib/utils.hpp
        lib/generators/euclidean_h_gen.hpp
        lib/generators/euclidean_phi_gen.hpp
        lib/generators/cosine_h_gen.hpp
        lib/generators/cosine_g_gen.hpp
        lib/data_structures/cust_hashtable.hpp
        lib/data_structures/vector_bucket.hpp
        lib/generators/euclidean_f_gen.hpp
        lib/generators/hypercube_gen.hpp
        lib/clustering_phases/initialization.hpp
        lib/clustering_phases/assignment.hpp
        lib/clustering_phases/silhouette.hpp
        lib/clustering_phases/update.hpp
        lib/lsh_cube.hpp)


add_executable(tests
        lib/in_out/arg_parser.cpp
        lib/in_out/arg_parser.h
        lib/generators/hash_generator.hpp
        lib/data_structures/cust_vector.hpp
        lib/in_out/vector_reader.hpp
        lib/utils.cpp
        lib/utils.hpp
        lib/generators/euclidean_h_gen.hpp
        lib/generators/euclidean_phi_gen.hpp
        lib/generators/cosine_h_gen.hpp
        lib/generators/cosine_g_gen.hpp
        lib/data_structures/cust_hashtable.hpp
        lib/data_structures/vector_bucket.hpp
        lib/generators/euclidean_f_gen.hpp
        lib/generators/hypercube_gen.hpp)