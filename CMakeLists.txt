cmake_minimum_required(VERSION 3.10)
project(Tweetoscope VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(Boost_USE_STATIC_LIBS ON)
# add_definitions(-DBOOST_LOG_DYN_LINK)
# add_definitions(-DBOOST_TEST_DYN_LINK)

find_package(PkgConfig)
find_package(Boost COMPONENTS unit_test_framework log REQUIRED)
find_package(Doxygen)
pkg_check_modules(GAML REQUIRED gaml)
pkg_check_modules(CPPKAFKA REQUIRED cppkafka)

if(NOT ENVIRONMENT)
    set(ENVIRONMENT development)
endif()

message("Using config for environment: ${ENVIRONMENT}")

if (NOT DOXYGEN_FOUND)
    message("Couldn't fond Doxygen. Documentation will not be generated")
else()
    add_subdirectory(doc)
endif()

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

link_libraries(${CPPKAFKA_LIBRARIES} pthread Boost::log_setup Boost::log)
include_directories(${CPPKAFKA_INCLUDE_DIRS} ${GAML_INCLUDE_DIRS})

add_subdirectory(src/cpp)
add_subdirectory(tests/cpp)
file(COPY src/python DESTINATION ${CMAKE_BINARY_DIR})
file(COPY tests/python DESTINATION ${CMAKE_BINARY_DIR}/tests)

add_custom_target(CopyConfig ALL)
file(GLOB CONFIG_FILES RELATIVE ${CMAKE_SOURCE_DIR}/config/${ENVIRONMENT} ${CMAKE_SOURCE_DIR}/config/${ENVIRONMENT}/*)
file(GLOB DEFAULT_CONFIG_FILES RELATIVE ${CMAKE_SOURCE_DIR}/config/default ${CMAKE_SOURCE_DIR}/config/default/*)

foreach(CONFIG_FILE ${CONFIG_FILES})
    add_custom_command(TARGET CopyConfig COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/config/${ENVIRONMENT}/${CONFIG_FILE} ${CMAKE_BINARY_DIR}/config/${CONFIG_FILE})
endforeach()

foreach(CONFIG_FILE ${DEFAULT_CONFIG_FILES})
    if(NOT CONFIG_FILE IN_LIST CONFIG_FILES )
        add_custom_command(TARGET CopyConfig COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/config/default/${CONFIG_FILE} ${CMAKE_BINARY_DIR}/config/${CONFIG_FILE})
    endif()
endforeach()

file(COPY data DESTINATION ${CMAKE_BINARY_DIR})
file(COPY scripts DESTINATION ${CMAKE_BINARY_DIR})

unset(ENVIRONMENT CACHE)
