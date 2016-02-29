set(SOURCE_EXE main.cpp)
set(SOURCE_LIB util.cpp queue.cpp util.h queue.h http.h http.cpp)

add_library(lib STATIC ${SOURCE_LIB})
add_executable(main ${SOURCE_EXE})

set(CMAKE_CXX_FLAGS "-std=c++0x")
target_link_libraries(main lib)

