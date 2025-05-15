cmake_minimum_required(VERSION 3.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(kieli)
set(CMAKE_BUILD_TYPE Debug)
find_package(LLVM REQUIRED CONFIG)
set(PCRE2_INCLUDE_DIRS "/usr/local/include")
set(PCRE2_LIBRARIES "/usr/local/lib/libpcre2-8.a")
include_directories(${LLVM_INCLUDE_DIRS} ${PCRE2_INCLUDE_DIRS})
set(LINK_LIBS
    LLVMCore
    LLVMSupport
    LLVMExecutionEngine 
    ${PCRE2_LIBRARIES}
)

include_directories(headers)

file(GLOB SOURCES "src/*.c")

file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})
add_executable(kieli ${SOURCES})
target_link_libraries(kieli ${LINK_LIBS})
