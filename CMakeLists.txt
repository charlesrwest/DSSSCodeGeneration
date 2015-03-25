cmake_minimum_required (VERSION 2.8.3)
PROJECT(DSSCodeGeneration)

#Get c++11
ADD_DEFINITIONS(-std=c++11)

find_package(Protobuf REQUIRED)


#set path to library
link_directories(/usr/lib/x86_64-linux-gnu)

#Set the binaries to be placed in the ./bin/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")

#Generate the C++ for the messages from the libprotobuf markup
add_subdirectory(./messages)

#Tell the subprojects where to find the created static library and headers
include_directories(./messages ./src/  )

link_directories(./messages)

#Tell cmake were to find the sub-projects
add_subdirectory(./src)

FILE(GLOB DSSCodeGeneratorSource ./src/*.cpp)

#Add the compilation target
ADD_EXECUTABLE(DSSSCodeGenerator ${DSSCodeGeneratorSource})

#link libraries to executable
target_link_libraries(DSSSCodeGenerator  ${PROTOBUF_LIBRARY} messages.a)

