cmake_minimum_required(VERSION 3.10.2)
project(simplehttplib C)

set(CMAKE_C_STANDARD 99)

add_library(simplehttplib STATIC httplib.c httputil.c httpparse.c httpser.c httplib.h httplib_internal.h)