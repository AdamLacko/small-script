cmake_minimum_required(VERSION 3.0.0)

project(small-compiler VERSION 2.3.1 LANGUAGES C)

add_executable(scpack scpack.c)
add_executable(sc sc1.c sc2.c sc3.c sc4.c sc5.c sc6.c sc7.c scvars.c sclist.c scexpand.c)