cmake_minimum_required(VERSION 3.3)
project(Adv_Compiler)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Scanner.cpp Scanner.h Helper.h FileReader.cpp FileReader.h Helper.cpp Parser.cpp Parser.h GraphDrawer.cpp GraphDrawer.h SSABuilder.cpp SSABuilder.h BasicBlock.cpp BasicBlock.h)
add_executable(Adv_Compiler ${SOURCE_FILES})