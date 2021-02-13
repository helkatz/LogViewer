project(core_test)

message("   - ${PROJECT_NAME}")

file(GLOB_RECURSE sources *.cpp *.h)

add_executable(core_test ${sources})
SOURCE_GROUP_BY_FOLDER(sources)

target_link_libraries(core_test core)
conan_target_link_libraries(${PROJECT_NAME})

set_target_properties (core_test PROPERTIES FOLDER tests)
