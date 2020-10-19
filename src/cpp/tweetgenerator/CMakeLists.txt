add_executable(tweetgenerator
    tweet-generator.cpp 
    ${CMAKE_BINARY_DIR}
    )

set_target_properties(tweetgenerator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

