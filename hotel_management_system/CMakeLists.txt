cmake_minimum_required(VERSION 3.20)
project(hotel)

set(CMAKE_CXX_STANDARD 17)


add_executable(hotel main.cpp room.cpp users.cpp)


add_executable(admin_registration admin_registration.cpp users.h)