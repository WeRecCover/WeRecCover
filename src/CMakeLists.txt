set(BINARY ${CMAKE_PROJECT_NAME}_run)
#set(LIBRARY ${CMAKE_PROJECT_NAME}_lib)

# log level, commented out = no logging (unused log statements should be automatically removed during compilation)
# uncomment to enable logging up to the defined level, available levels: trace, debug, info, warning, error, fatal
#add_definitions(-DCOVER_MAX_LOG_LEVEL=debug)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS thread chrono log_setup log)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_Declare(
    flat_hash_map
    GIT_REPOSITORY https://github.com/skarupke/flat_hash_map.git
    GIT_TAG 2c4687431f978f02a3780e24b8b701d22aa32d9c
    )
FetchContent_MakeAvailable(json)

FetchContent_Populate(flat_hash_map)
include_directories(${flat_hash_map_SOURCE_DIR} ${flat_hash_map_BINARY_DIR})

option(CXX "enable C++ compilation" ON)

if (CXX)
    enable_language(CXX)
endif ()

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES instance.cpp rectangle.cpp datastructures.h
    algorithm.h algorithm_runner.cpp strip_algorithm.cpp strip_algorithm.h
    baserect_graph.h baserect_graph.cpp
    strip_algorithm.h strip_algorithm.cpp
    cover_splitter.h partition_algorithm.cpp partition_algorithm.h util.cpp util.h bbox_cover_splitter.cpp
    bbox_cover_splitter.h cover_splitter.cpp partition_cover_splitter.cpp partition_cover_splitter.h
    rectangle_enumerator.cpp rectangle_enumerator.h greedy_set_cover_algorithm.cpp greedy_set_cover_algorithm.h
    ILP_algorithm.cpp ILP_algorithm.h CGAL_classes.h cover_provider.h cover_postprocessor.h cover_postprocessor.cpp 
    cover_pruner.cpp cover_pruner.h cover_trimmer.cpp cover_trimmer.h cover_joiner.cpp cover_joiner.h logging.h 
    cover_joiner_full.cpp cover_joiner_full.h result_writer.cpp result_writer.h
    )

add_executable(${BINARY} ${SOURCE_FILES} main.cpp)

find_package(GUROBI)

if (GUROBI_FOUND)
    message("Gurobi found, including ILP formulation")
    add_definitions(-DGUROBI_AVAILABLE)
    include_directories(${GUROBI_INCLUDE_DIRS})
else ()
    message("Gurobi not found, not including ILP formulation")
endif ()

if (GUROBI_FOUND AND CXX)
    target_link_libraries(${BINARY} PUBLIC optimized ${GUROBI_CXX_LIBRARY}
            debug ${GUROBI_CXX_DEBUG_LIBRARY})
endif ()

target_link_libraries(${BINARY} ${CGAL_LIBS})
target_link_libraries(${BINARY} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${BINARY} PUBLIC Boost::thread Boost::chrono Boost::log Boost::log_setup)

if (GUROBI_FOUND)
    target_link_libraries(${BINARY} PUBLIC ${GUROBI_LIBRARY})
endif ()

target_link_libraries(${BINARY} PUBLIC CLI11::CLI11)

# SOURCE BEGIN https://vicrucann.github.io/tutorials/quick-cmake-doxygen/
if (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")
    # first we can indicate the documentation build as an option and set it to ON by default
    option(BUILD_DOC "Build documentation" ON)

    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target(doc_doxygen ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM)
    else (DOXYGEN_FOUND)
        message("Doxygen needs to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif ()
# SOURCE END
