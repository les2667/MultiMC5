# run the unit tests with `make test`
find_package(Qt5 REQUIRED COMPONENTS Test Core Network)

include_directories(${MMC_SRC} ${CMAKE_CURRENT_SOURCE_DIR} ${MMC_SRC}/logic ${MMC_SRC}/depends/util/include)

# Tests START #

# a test is a file named tst_<name>.cpp anywhere in a subdirectory of the current directory
# it can contain an include for ModelTester.h and/or TestUtil.h, in which case they will be
# compiled into the executable
set(tests
	logic/auth/tst_AccountModel.cpp
	logic/auth/tst_DefaultAccountProxyModel.cpp
	logic/minecraft/auth/tasks/tst_AuthenticateTask.cpp
	logic/minecraft/auth/tasks/tst_RefreshTask.cpp
	logic/minecraft/auth/tasks/tst_ValidateTask.cpp
	logic/minecraft/auth/tst_MojangAccount.cpp
	logic/minecraft/tst_GradleSpecifier.cpp
	logic/resources/tst_Resource.cpp
	logic/screenshots/auth/tst_ImgurAccount.cpp
	logic/settings/tst_INIFile.cpp
	logic/updater/tst_DownloadTask.cpp
	logic/updater/tst_UpdateChecker.cpp
)

set(data_files
	data/1.json
	data/2.json
	data/accounts_v2.json
	data/accounts_v3.json
	data/channels.json
	data/errorChannels.json
	data/fileOneA
	data/fileOneB
	data/fileThree
	data/fileTwo
	data/garbageChannels.json
	data/index.json
	data/noChannels.json
	data/oneChannel.json
	data/tst_DownloadTask-test_writeInstallScript.xml
	data/tst_userutils-test_createShortcut-unix
)

set(binary_files
)

foreach(test ${tests})
	# Extract the name from the source file
	get_filename_component(name "${test}" NAME_WE)
	string(REPLACE "tst_" "" name "${name}")

	# Gather a list of source files
	set(src ${test} TestUtil.h)
	file(READ "${test}" test_contents)
	if("${test_contents}" MATCHES "ModelTester\\.h")
		list(APPEND src ModelTester.h)
	elseif("${test_contents}" MATCHES "AccountTester\\.h")
		list(APPEND src AccountTester.h ${CMAKE_BINARY_DIR}/application/BuildConfig.cpp)
	endif()

	# Extra file needed on windows
	if(WIN32)
		list(APPEND src ${CMAKE_CURRENT_SOURCE_DIR}/test.rc)
	endif()

	# Create and set up the executable
	add_executable(tst_${name} ${src})
	target_link_libraries(tst_${name} Qt5::Core Qt5::Test Qt5::Network MultiMC_logic)
	add_test(NAME ${name} COMMAND tst_${name})
endforeach()

# Tests END #

set(MultiMC_TEST_DATA_PATH "${CMAKE_CURRENT_BINARY_DIR}/data")
if(UNIX)
	# on unix we get the third / from the filename
	set(MultiMC_TEST_DATA_PATH "file://${MultiMC_TEST_DATA_PATH}")
else()
	# we don't on windows, so we have to add it ourselves
	set(MultiMC_TEST_DATA_PATH "file:///${MultiMC_TEST_DATA_PATH}")
endif()

foreach(data_file ${data_files})
	if (NOT IS_DIRECTORY ${data_file})
		get_filename_component(filename ${data_file} NAME)
		configure_file(
			${data_file}
			${CMAKE_CURRENT_BINARY_DIR}/data/${filename}
			@ONLY
			NEWLINE_STYLE LF
		)
	endif()
endforeach()

foreach(binary_file ${binary_files})
	get_filename_component(filename ${binary_file} NAME)
	configure_file(${binary_file} ${CMAKE_CURRENT_BINARY_DIR}/data/${filename} COPYONLY)
endforeach()

configure_file(test_config.h.in test_config.h @ONLY)
add_custom_target(dummy_extra_test_headers SOURCES TestUtil.h ModelTester.h)
