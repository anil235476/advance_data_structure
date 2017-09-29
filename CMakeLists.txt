cmake_minimum_required (VERSION 3.2)



#project name 
project (driver)

include_directories(${driver_SOURCE_DIR}/inc)

add_executable(driver
	main.cxx
	inc/array_view.h
)

