# required CapnProto
include(GNUInstallDirs)
set(CAPNPROTO_PATH ${SRC_BACKENDS}/SynchroTraceGen)
add_compile_options(-O0 -g)
execute_process(
	COMMAND git submodule update --init ${CAPNPROTO_PATH}/capnproto
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

include(ExternalProject)
ExternalProject_Add(capnproto
	PREFIX capnproto
# -- Download step ------------
# -- Update step --------------
# -- Configure step -----------
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/capnproto/c++
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${THIRD_PARTY}/capnproto
	-DBUILD_TOOLS=OFF
	-DBUILD_TESTING=OFF
# -- Build step ---------------
# -- Install step -------------
	)
include_directories(${THIRD_PARTY}/capnproto/include)
include_directories(STEventTraceSchemas)

# SynchroTraceGen Backend
set(SOURCES
	EventHandlers.cpp
	ThreadContext.cpp
	TextLogger.cpp
	TextLoggerV2.cpp
	CapnLogger.cpp
	STEvent.cpp
	STEventTraceSchemas/STEventTraceCompressed.capnp.c++
	STEventTraceSchemas/STEventTraceUncompressed.capnp.c++
	${THIRD_PARTY}/zlib/contrib/iostream3/zfstream.cc)
add_library(STGenCore STATIC ${SOURCES})

# Link CapnProto w/ SynchroTraceGen statically
set(KJ_LIB ${THIRD_PARTY}/capnproto/${CMAKE_INSTALL_LIBDIR}/libkj.a)
set(CAPNP_LIB ${THIRD_PARTY}/capnproto/${CMAKE_INSTALL_LIBDIR}/libcapnp.a)
set(STGEN_LIB ${CMAKE_CURRENT_BINARY_DIR}/libSTGen.a)
add_custom_target(STGen ALL
	COMMAND ar -x $<TARGET_FILE:STGenCore>
	COMMAND ar -x ${KJ_LIB}
	COMMAND ar -x ${CAPNP_LIB}
	COMMAND ar -qc ${STGEN_LIB} *.o)
add_dependencies(STGenCore capnproto)
add_dependencies(STGen STGenCore)

# tests
add_subdirectory(tests)

set(PRISM_TOOL_DEPENDENCIES STGen PARENT_SCOPE)
set(PRISM_TOOL_LINK_LIBS ${STGEN_LIB};z PARENT_SCOPE)
