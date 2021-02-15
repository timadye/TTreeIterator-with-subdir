file(GLOB HEADERS TTreeIterator/*.h)

ROOT_GENERATE_DICTIONARY(G__TTreeIterator ${HEADERS} LINKDEF src/LinkDef.h)

file(GLOB SOURCES src/*.cxx)

add_library(TTreeIterator SHARED ${SOURCES} G__TTreeIterator )
target_link_libraries(TTreeIterator ${ROOT_LIBRARIES})

target_sources(TTreeIterator PRIVATE ${CMAKE_BINARY_DIR}/versioning/TTreeIteratorVersion.h)
set_source_files_properties(${CMAKE_BINARY_DIR}/versioning/TTreeIteratorVersion.h PROPERTIES GENERATED TRUE)
target_include_directories(TTreeIterator PRIVATE ${CMAKE_BINARY_DIR}/versioning)
target_include_directories(TTreeIterator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(TTreeIterator TTreeIterator_version)


enable_testing()
include(GoogleTest)
add_executable(TestTrees test/treeTests.cxx)
target_link_libraries(TestTrees TTreeIterator gtest gtest_main)
add_custom_command(TARGET TestTrees
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/test/model.root"
        "$<TARGET_FILE_DIR:TestTrees>/model.root")

install( DIRECTORY TTreeIterator DESTINATION include FILES_MATCHING
        COMPONENT headers
        PATTERN "*.h"
        )

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/libTTreeIterator.rootmap
        ${CMAKE_CURRENT_BINARY_DIR}/libTTreeIterator_rdict.pcm
        DESTINATION lib
        COMPONENT libraries)

install(TARGETS TTreeIterator
        LIBRARY DESTINATION lib
        COMPONENT libraries)