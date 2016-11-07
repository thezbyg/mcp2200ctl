cmake_minimum_required(VERSION 3.1)
project(mcp2200ctl)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
file(GLOB SOURCES src/*.cpp src/*.h)
file(GLOB CONSOLE_SOURCES src/console/*.cpp src/console/*.h)
file(GLOB GUI_SOURCES src/gui/*.cpp src/gui/*.h)
if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-Wl,--as-needed")
endif(CMAKE_COMPILER_IS_GNUCXX)

if ("${CMAKE_SYSTEM_NAME}" MATCHES Linux)
	add_definitions(-DLINUX_BUILD)
	file(GLOB GUI_UDEV_SOURCES src/gui/udev/*.cpp src/gui/udev/*.h)
endif ("${CMAKE_SYSTEM_NAME}" MATCHES Linux)

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

find_package(Boost 1.58 COMPONENTS program_options filesystem unit_test_framework REQUIRED)
find_package(Hidapi REQUIRED)
if (BUILD_GUI)
	find_package(PkgConfig)
	if (PkgConfig_FOUND)
		pkg_check_modules(GTK3 gtk+-3.0)
		pkg_check_modules(Libudev libudev)
		pkg_check_modules(Jsoncpp jsoncpp)
	endif (PkgConfig_FOUND)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
endif (BUILD_GUI)

add_library(mcp2200 ${SOURCES})
target_compile_options(mcp2200 PUBLIC -std=c++11 -Wall -Wfloat-conversion)
target_include_directories(mcp2200 PUBLIC src)
target_link_libraries(mcp2200 PUBLIC
	${Boost_LIBRARIES}
	${Hidapi_LIBRARIES}
)
target_include_directories(mcp2200 PUBLIC
	${Boost_INCLUDE_DIRS}
	${Hidapi_INCLUDE_DIRS}
)

add_executable(test_gpio_mask src/test/gpio_mask.cpp)
target_compile_options(test_gpio_mask PUBLIC -std=c++11 -Wall -Wfloat-conversion)
target_link_libraries(test_gpio_mask PUBLIC mcp2200)
target_include_directories(test_gpio_mask PUBLIC src src/test)
target_link_libraries(test_gpio_mask PUBLIC
	${Boost_LIBRARIES}
	${Hidapi_LIBRARIES}
)
target_include_directories(test_gpio_mask PUBLIC
	${Boost_INCLUDE_DIRS}
	${Hidapi_INCLUDE_DIRS}
)

if (BUILD_CTL)
	add_executable(mcp2200ctl ${CONSOLE_SOURCES})
	target_compile_options(mcp2200ctl PUBLIC -std=c++11 -Wall -Wfloat-conversion)
	target_link_libraries(mcp2200ctl PUBLIC mcp2200)
	target_include_directories(mcp2200ctl PUBLIC src src/console)
	target_link_libraries(mcp2200ctl PUBLIC
		${Boost_LIBRARIES}
		${Hidapi_LIBRARIES}
	)
	target_include_directories(mcp2200ctl PUBLIC
		${Boost_INCLUDE_DIRS}
		${Hidapi_INCLUDE_DIRS}
	)
	install(TARGETS mcp2200ctl DESTINATION bin)
endif (BUILD_CTL)

if (BUILD_GUI AND GTK3_FOUND AND Libudev_FOUND AND Jsoncpp_FOUND)
	add_executable(mcp2200gui ${GUI_SOURCES} ${GUI_UDEV_SOURCES})
	target_compile_options(mcp2200gui PUBLIC -std=c++11 -Wall -Wfloat-conversion)
	target_link_libraries(mcp2200gui PUBLIC mcp2200)
	target_include_directories(mcp2200gui PUBLIC src src/gui)
	target_link_libraries(mcp2200gui PUBLIC
		${Boost_LIBRARIES}
		${Hidapi_LIBRARIES}
		${GTK3_LIBRARIES}
		${Libudev_LIBRARIES}
		${Jsoncpp_LIBRARIES}
		Threads::Threads
	)
	target_include_directories(mcp2200gui PUBLIC
		${Boost_INCLUDE_DIRS}
		${Hidapi_INCLUDE_DIRS}
		${GTK3_INCLUDE_DIRS}
		${Libudev_INCLUDE_DIRS}
		${Jsoncpp_INCLUDE_DIRS}
	)
	install(TARGETS mcp2200gui DESTINATION bin)
endif (BUILD_GUI AND GTK3_FOUND AND Libudev_FOUND AND Jsoncpp_FOUND)

