find_package(Log4Cpp REQUIRED)
include_directories(${Log4Cpp_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Log4Cpp_LIBRARIES})

add_executable (
	harmonyhue
	main.cpp
	logger.cpp logger.h
	csocket.cpp csocket.h
	base64.cpp base64.h
	curl.cpp curl.h
	hparser.cpp hparser.h hprinter.cpp 
	hstates.cpp hstates.h
	config.cpp config.h
	authorization.cpp authorization.h
)

target_link_libraries(harmonyhue ${LIBS})

install(TARGETS harmonyhue DESTINATION bin)
