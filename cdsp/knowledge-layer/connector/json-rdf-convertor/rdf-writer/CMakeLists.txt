# Define the rdf-writer library
add_library(rdf_writer
    src/triple_assembler.cpp
    src/triple_writer.cpp
    ${PROJECT_ROOT_DIR}/connector/utils/file_handler_impl.cpp
    ${PROJECT_ROOT_DIR}/connector/utils/helper.cpp
    ${PROJECT_ROOT_DIR}/connector/utils/coordinate_transform.cpp
)

# Include directories
target_include_directories(rdf_writer 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${serd_SOURCE_DIR}/include
        ${PROJECT_ROOT_DIR}/connector/utils
        ${PROJECT_ROOT_DIR}/third_party/GeographicLib/include
)

# Link libraries
target_link_libraries(rdf_writer
    PUBLIC 
        rdfox
        ${SERD_LIBRARIES} 
    PRIVATE 
        nlohmann_json::nlohmann_json
        geographiclib
)