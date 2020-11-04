add_executable(TweetCollector
    main.cpp 
    ${CMAKE_BINARY_DIR}
    )

set_target_properties(TweetCollector PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )

