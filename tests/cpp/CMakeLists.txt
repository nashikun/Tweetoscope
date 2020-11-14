include_directories(${CMAKE_SOURCE_DIR}/src/cpp)

add_executable(CollectorTest
    CollectorTest.cpp 
    ${CMAKE_BINARY_DIR}/tests
    )

target_link_libraries(CollectorTest
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      )
