cmake_minimum_required(VERSION 3.0)
project(mcp2200ctl)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

find_program(GIT_EXECUTABLE git DOC "Git version control")
mark_as_advanced(GIT_EXECUTABLE)
find_file(GITDIR NAMES .git PATHS ${CMAKE_SOURCE_DIR} NO_DEFAULT_PATH)
find_file(VERSION_FILE NAMES version.cpp PATHS ${CMAKE_SOURCE_DIR}/cmake/ NO_DEFAULT_PATH)

set(GENERATE_VERSION true)
if (GIT_EXECUTABLE AND GITDIR)
	execute_process(COMMAND
		"${GIT_EXECUTABLE}" describe "--match=v*" --always
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		OUTPUT_VARIABLE VERSION_INFO
		ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
	)
elseif (VERSION_FILE)
	list(APPEND SOURCES "${VERSION_FILE}")
	set(GENERATE_VERSION false)
else()
	set(VERSION_INFO "v0.0.0")
endif()

if (GENERATE_VERSION)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" @ONLY)
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")
endif()

add_executable(mcp2200ctl ${SOURCES})
target_compile_options(mcp2200ctl PUBLIC -std=c++11 -Wall -Wfloat-conversion)
target_link_libraries(mcp2200ctl PUBLIC -Wl,--as-needed)
target_include_directories(mcp2200ctl PUBLIC src)
find_package(Boost 1.58 COMPONENTS program_options REQUIRED)
find_package(Hidapi REQUIRED)
target_link_libraries(mcp2200ctl PUBLIC
	${Boost_LIBRARIES}
	${Hidapi_LIBRARIES}
)
target_include_directories(mcp2200ctl PUBLIC
	${Boost_INCLUDE_DIRS}
	${Hidapi_INCLUDE_DIRS}
)
install(TARGETS mcp2200ctl DESTINATION bin)
