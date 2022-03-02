cmake_minimum_required(VERSION 3.0)

project(CGameEngine)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(CGameEngine STATIC
	CGameEngine.h
)