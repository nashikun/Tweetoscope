configure_file(Doxyfile.in Doxyfile)
set(DOXYGEN_INPUT ${CMAKE_BINARY_DIR}/doc/Doxyfile)
add_custom_target(doc ALL 
    COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation.."
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_INPUT} > /dev/null
    COMMAND ${CMAKE_COMMAND} -E echo "Doc built successfully"
    )
