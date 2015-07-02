cmake_minimum_required(VERSION 2.8)
set(CMAKE_INSTALL_PREFIX $ENV{HOME})
project(FbyNetCPP)
add_definitions(-std=c++11 -Wall -Wextra -Weffc++)

# -Weffc++


# enable_testing()
# add_test( testname Exename arg1 arg2 ... )
#
# Or, in its longer form:
#
#   add_test(NAME <name> [CONFIGURATIONS [Debug|Release|...]]
#            [WORKING_DIRECTORY dir]
#            COMMAND <command> [arg1 [arg2 ...]])
#
# http://www.cmake.org/Wiki/CMake/Testing_With_CTest






set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)


#FIND_LIBRARY(GTEST_LIBRARY gtest /home/danlyke/lib)
#TARGET_LINK_LIBRARIES(testOVAS ${GTEST_LIBRARY})
#add_library( fby STATIC IMPORTED )

LINK_DIRECTORIES( ${LINK_DIRECTORIES} $ENV{HOME}/lib)
include_directories($ENV{HOME}/include)

# target_link_libraries(fby LINK_PUBLIC sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams )


add_custom_command( OUTPUT sqldefinitions.cpp sqldefinitions.sql
  COMMAND sqlextractor ARGS ${CMAKE_CURRENT_SOURCE_DIR}/wikiobjects.h
  MAIN_DEPENDENCY wikiobjects.h )

add_custom_command( OUTPUT openlayerstemplate.cpp openlayerstemplate.h
  COMMAND fixedstringextractor ARGS sections ${CMAKE_CURRENT_SOURCE_DIR}/openlayerstemplate.js openlayerstemplate
  MAIN_DEPENDENCY openlayerstemplate.js )


add_library( fbywiki sqldefinitions.cpp wiki.cpp wikidb.cpp wikiobjects.cpp wikinode.cpp wikivideoflash.cpp wikistatusupdate.cpp  wikiopenlayers.cpp wikidpl.cpp openlayerstemplate.cpp )


add_executable(fby-bin fby.cpp)
set_target_properties(fby-bin
  PROPERTIES OUTPUT_NAME fby)
target_link_libraries (fby-bin LINK_PUBLIC fbywiki fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(test_microformats test_microformats.cpp)
target_link_libraries (test_microformats LINK_PUBLIC fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(test_lib test_lib.cpp)
target_link_libraries (test_lib LINK_PUBLIC fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(test_fbydb test_fbydb.cpp)
target_link_libraries (test_fbydb LINK_PUBLIC fby fbywiki sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(fby_test fby_test.cpp)
target_link_libraries (fby_test LINK_PUBLIC fbywiki fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(scaninputwikifiles scaninputwikifiles.cpp)
target_link_libraries (scaninputwikifiles LINK_PUBLIC fbywiki fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(scanimages scanimages.cpp)
target_link_libraries (scanimages LINK_PUBLIC fbywiki fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(parsewikifiletostaging parsewikifiletostaging.cpp)
target_link_libraries (parsewikifiletostaging LINK_PUBLIC fbywiki fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(parsewikifilesforlinks parsewikifilesforlinks.cpp)
target_link_libraries (parsewikifilesforlinks LINK_PUBLIC fbywiki fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(scanwikifilesforlinks scanwikifilesforlinks.cpp)
target_link_libraries (scanwikifilesforlinks LINK_PUBLIC fbywiki fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(rebuilddirtyfiles rebuilddirtyfiles.cpp)
target_link_libraries (rebuilddirtyfiles LINK_PUBLIC fbywiki fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(getstatus getstatus.cgi.cpp)
target_link_libraries (getstatus LINK_PUBLIC fbywiki fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

add_executable(setstatus setstatus.cgi.cpp)
target_link_libraries (setstatus LINK_PUBLIC fbywiki fby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a /home/danlyke/lib/libgtest_main.a pthread)

