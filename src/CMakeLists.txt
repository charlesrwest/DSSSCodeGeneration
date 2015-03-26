 #Set the binaries to be placed in the ./bin/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin/")

FILE(GLOB DSSCodeGeneratorSource ./*.cpp)

#Add the compilation target
ADD_EXECUTABLE(DSSSCodeGenerator ${DSSCodeGeneratorSource})

#link libraries to executable
target_link_libraries(DSSSCodeGenerator  ${PROTOBUF_LIBRARY} messages.a)
