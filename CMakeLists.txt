cmake_minimum_required (VERSION 2.8.11)
project (umbra-debugger)

set(SOURCES main.cpp)
add_executable(umbra-debugger ${SOURCES})

set(CPPDAP_DIR "${CMAKE_SOURCE_DIR}/cppdap")
set(CPPDAP_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set(CPPDAP_JSON_DIR        ${CPPDAP_THIRD_PARTY_DIR}/json)
set(CPPDAP_GOOGLETEST_DIR  ${CPPDAP_THIRD_PARTY_DIR}/googletest)
add_subdirectory(${CPPDAP_DIR})
target_link_libraries(umbra-debugger cppdap)
