cmake_minimum_required(VERSION 2.6)

set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(harmonyhueconnect)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpermissive -Wno-deprecated-declarations")

find_package(GLIBMM2 REQUIRED)
include_directories(${GLIBMM2_INCLUDE_DIR})
set(LIBS ${LIBS} ${GLIBMM2_LIBRARY})

find_package(LibXML++ REQUIRED)
include_directories(${LibXML++_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LibXML++_LIBRARIES})

find_package(Jansson REQUIRED)
include_directories(${JANSSON_INCLUDE_DIRS})
set(LIBS ${LIBS} ${JANSSON_LIBRARIES})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
set(LIBS ${LIBS} ${CURL_LIBRARIES})

add_executable(
	harmonyhueconnect
	HarmonyHubControl.cpp 
	initdaemon.cpp initdaemon.h 
	tinyxml2.cpp tinyxml2.h 
	csocket.cpp csocket.h 
	myparser.cpp myparser.h 
	libxmlppparsermain.cpp
	parsejson.cpp parsejson.h
	hue.cpp hue.h)
	
target_link_libraries(harmonyhueconnect ${LIBS})

install(TARGETS harmonyhueconnect RUNTIME DESTINATION bin)
