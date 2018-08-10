project(m2-file-logger)
cmake_minimum_required(VERSION 2.8)

link_directories(${CMAKE_SOURCE_DIR}/vendor/m2sdk/lib)
include_directories(vendor/m2sdk/include)
include_directories(vendor/include)

add_library(client SHARED logger.cpp)
target_link_libraries(client m2sdk "-SAFESEH:NO")

set_target_properties(client PROPERTIES LINK_FLAGS_DEBUG "/SAFESEH:NO")
set_target_properties(client PROPERTIES LINK_FLAGS_RELEASE "/SAFESEH:NO")

add_custom_command(TARGET client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/client.dll" "${CMAKE_SOURCE_DIR}/binary/"
)
