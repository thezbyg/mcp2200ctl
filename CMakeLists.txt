cmake_minimum_required(VERSION 3.13)
project(mcp2200ctl)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
file(GLOB SOURCES src/*.cpp src/*.h)
file(GLOB CONSOLE_SOURCES src/console/*.cpp src/console/*.h)
file(GLOB GUI_SOURCES src/gui/*.cpp src/gui/*.h)
if ("${CMAKE_SYSTEM_NAME}" MATCHES Linux)
	file(GLOB GUI_UDEV_SOURCES src/gui/udev/*.cpp src/gui/udev/*.h)
endif()

option(BUILD_CTL "build console program" TRUE)
option(BUILD_GUI "build GTK3 based GUI program" TRUE)

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

find_package(Boost 1.67 COMPONENTS program_options unit_test_framework REQUIRED)
find_package(Hidapi REQUIRED)
if (BUILD_GUI)
	find_package(PkgConfig)
	if (PkgConfig_FOUND)
		pkg_check_modules(GTK3 gtk+-3.0)
		pkg_check_modules(Libudev libudev)
		pkg_check_modules(Jsoncpp jsoncpp)
	endif()
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
endif()

function(setCompileOptions target)
	if (MSVC)
		target_compile_options(${target} PRIVATE /MD /utf-8)
		target_compile_definitions(${target} PRIVATE UNICODE)
	else()
		target_compile_options(${target} PRIVATE -Wall -Wextra -Wfloat-conversion)
	endif()
	target_compile_features(${target} PRIVATE cxx_std_17)
	target_link_libraries(${target} PRIVATE
		${Boost_LIBRARIES}
		${Hidapi_LIBRARIES}
	)
	target_include_directories(${target} PRIVATE
		src
		${Boost_INCLUDE_DIRS}
		${Hidapi_INCLUDE_DIRS}
	)
	target_compile_definitions(${target} PRIVATE
		${Boost_DEFINITIONS}
		${Hidapi_DEFINITIONS}
	)
	if ("${CMAKE_SYSTEM_NAME}" MATCHES Linux)
		target_compile_definitions(${target} PRIVATE LINUX_BUILD)
	endif()
	if (CMAKE_COMPILER_IS_GNUCXX)
		target_link_options(${target} PRIVATE "-Wl,--as-needed")
	endif()
	set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endfunction()

add_library(mcp2200 ${SOURCES})
setCompileOptions(mcp2200)
file(GLOB sources src/test/*.cpp)
add_executable(tests ${sources})
setCompileOptions(tests)
target_link_libraries(tests PRIVATE mcp2200)
target_include_directories(tests PRIVATE src/test)

if (BUILD_CTL)
	add_executable(mcp2200ctl ${CONSOLE_SOURCES})
	setCompileOptions(mcp2200ctl)
	target_link_libraries(mcp2200ctl PRIVATE mcp2200)
	target_include_directories(tests PRIVATE src/console)
	install(TARGETS mcp2200ctl DESTINATION bin)
endif()

if (BUILD_GUI AND GTK3_FOUND AND Libudev_FOUND AND Jsoncpp_FOUND)
	add_executable(mcp2200gui ${GUI_SOURCES} ${GUI_UDEV_SOURCES})
	setCompileOptions(mcp2200gui)
	target_link_libraries(mcp2200gui PUBLIC
		mcp2200
		${GTK3_LIBRARIES}
		${Libudev_LIBRARIES}
		${Jsoncpp_LIBRARIES}
		Threads::Threads
	)
	target_include_directories(mcp2200gui PUBLIC
		src/gui
		${GTK3_INCLUDE_DIRS}
		${Libudev_INCLUDE_DIRS}
		${Jsoncpp_INCLUDE_DIRS}
	)
	install(TARGETS mcp2200gui DESTINATION bin)
endif()

