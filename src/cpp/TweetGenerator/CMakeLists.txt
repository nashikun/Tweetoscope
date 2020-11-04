add_executable(TweetGenerator
    main.cpp 
    ${CMAKE_BINARY_DIR}
    )

set_target_properties(TweetGenerator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

