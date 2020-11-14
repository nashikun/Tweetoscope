cmake_minimum_required(VERSION 3.10)
project(Tweetoscope VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
pkg_check_modules(GAML REQUIRED gaml)
pkg_check_modules(CPPKAFKA REQUIRED cppkafka)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

link_libraries(${CPPKAFKA_LIBRARIES} pthread)
include_directories(${CPPKAFKA_INCLUDE_DIRS} ${GAML_INCLUDE_DIRS})

add_subdirectory(src/cpp)
add_subdirectory(tests/cpp)
file(COPY src/python DESTINATION ${CMAKE_BINARY_DIR})
file(COPY tests/python DESTINATION ${CMAKE_BINARY_DIR}/tests)
file(COPY config DESTINATION ${CMAKE_BINARY_DIR})
file(COPY data DESTINATION ${CMAKE_BINARY_DIR})
file(COPY scripts DESTINATION ${CMAKE_BINARY_DIR})

