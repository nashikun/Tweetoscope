cmake_minimum_required(VERSION 3.10)
project(Tweetoscope_2020_06 VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message("Linking the libraries")
set(Boost_USE_STATIC_LIBS ON)
find_package(PkgConfig)
find_package(Boost COMPONENTS unit_test_framework log REQUIRED)
find_package(Doxygen)
find_package(CppKafka REQUIRED)
find_program(Python "python3" REQUIRED)
pkg_check_modules(GAML REQUIRED gaml)
pkg_check_modules(RDKAFKA REQUIRED rdkafka)

if (NOT DOXYGEN_FOUND)
    message("Couldn't fond Doxygen. Documentation will not be generated")
else()
    add_subdirectory(doc)
endif()

# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

link_libraries(CppKafka::cppkafka pthread Boost::log_setup Boost::log ${RDKAFKA_STATIC_LDFLAGS})
include_directories(${GAML_INCLUDE_DIRS})

add_custom_target(CopyConfig ALL)

message("Copying the python files")
file(COPY requirements.txt DESTINATION ${CMAKE_BINARY_DIR})
file(COPY src/ml DESTINATION ${CMAKE_BINARY_DIR})
file(COPY tests/python DESTINATION ${CMAKE_BINARY_DIR}/tests)
file(READ requirements.txt PYTHON_REQUIREMENTS)
configure_file(${CMAKE_SOURCE_DIR}/setup.py ${CMAKE_BINARY_DIR}/setup.py)

message("Copying the C++ files")
add_subdirectory(src/cpp)
add_subdirectory(tests/cpp)

foreach(ENVIRONMENT development test deployment)
        file(GLOB CONFIG_FILES RELATIVE ${CMAKE_SOURCE_DIR}/config/${ENVIRONMENT} ${CMAKE_SOURCE_DIR}/config/${ENVIRONMENT}/*)
        file(GLOB DEFAULT_CONFIG_FILES RELATIVE ${CMAKE_SOURCE_DIR}/config/default ${CMAKE_SOURCE_DIR}/config/default/*)

    foreach(CONFIG_FILE ${CONFIG_FILES})
        add_custom_command(TARGET CopyConfig COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/config/${ENVIRONMENT}/${CONFIG_FILE} ${CMAKE_BINARY_DIR}/config/${ENVIRONMENT}/${CONFIG_FILE})
    endforeach()

    foreach(CONFIG_FILE ${DEFAULT_CONFIG_FILES})
        if(NOT CONFIG_FILE IN_LIST CONFIG_FILES )
            add_custom_command(TARGET CopyConfig COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/config/default/${CONFIG_FILE} ${CMAKE_BINARY_DIR}/config/${ENVIRONMENT}/${CONFIG_FILE})
        endif()
    endforeach()

endforeach()

message("Copying the remaining files")
file(COPY data DESTINATION ${CMAKE_BINARY_DIR})
file(COPY scripts DESTINATION ${CMAKE_BINARY_DIR})

add_custom_target(SetupPython ALL)
add_custom_command(
    TARGET SetupPython
    COMMAND ${CMAKE_COMMAND} -E echo "Building the python package"
)
add_custom_command(
    TARGET SetupPython
    COMMAND ${Python} ARGS setup.py sdist
)
