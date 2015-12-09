cmake_minimum_required(VERSION 3.1)

project(ParserMonad CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
  .
  GSL/include
)

add_executable (ParserMonad main.cc parser.h parser_monad.h)

