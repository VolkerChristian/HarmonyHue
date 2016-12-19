find_package(Log4Cpp REQUIRED)
include_directories(${Log4Cpp_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Log4Cpp_LIBRARIES})

find_package(LibXml2)
set(LIBS ${LIBS} ${LIBXML2_LIBRARIES})
include_directories(${LIBXML2_INCLUDE_DIR})

add_definitions(-DCONFIG_FILE="/etc/harmonyhue.conf")

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
	myxmltextreader.cpp myxmltextreader.h
	harmonyhue.service
)

target_link_libraries(harmonyhue ${LIBS})

install(TARGETS harmonyhue DESTINATION bin)

configure_file(harmonyhue.service.in harmonyhue.service)
install(FILES harmonyhue.service DESTINATION etc/systemd/system)
