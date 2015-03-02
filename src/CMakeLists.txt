cmake_minimum_required(VERSION 2.8)
project(FbyNetCPP)
add_definitions(-std=c++11 -Wall -Wextra)
# -Weffc++

add_library(libfby fbystd.cpp parsertokens.cpp textparse.yy.cpp fbystring.cpp fbyparsetree.cpp fbyregex.cpp fby_core_unix_array.cpp fby_core_unix_helpers.cpp fby_core_unix.cpp fbydb.cpp fbydb_sqlite3.cpp fbydb_postgresql.cpp fbyregexparser.cpp fbytreebuilder.cpp fbywikistrings.cpp fbynetserver.cpp fbyfileutils.cpp fbyimageutils.cpp fbyfilefind.cpp)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

#FIND_LIBRARY(GTEST_LIBRARY gtest /home/danlyke/lib)
#TARGET_LINK_LIBRARIES(testOVAS ${GTEST_LIBRARY})
#LINK_DIRECTORIES( ${LINK_DIRECTORIES} /home/danyke/lib)
include_directories(/home/danlyke/include)




add_executable(test_image_name_decomposition test_image_name_decomposition.cpp)
target_link_libraries (test_image_name_decomposition LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(test_lib test_lib.cpp)
target_link_libraries (test_lib LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable( test_subst  test_subst.cpp)
target_link_libraries ( test_subst LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(test_zstring test_zstring.cpp)
target_link_libraries (test_zstring LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(test_removecrs test_removecrs.cpp)
target_link_libraries (test_removecrs LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(test_wikistring test_wikistring.cpp)
target_link_libraries (test_wikistring LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(example_file example_file.cpp)
target_link_libraries (example_file LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(example_lambda example_lambda.cpp)
target_link_libraries (example_lambda LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(example_flex example_flex.cpp)
target_link_libraries (example_flex LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(example_tester example_tester.cpp)
target_link_libraries (example_tester LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(example_reset example_reset.cpp)
target_link_libraries (example_reset LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(example_regex example_regex.cpp)
target_link_libraries (example_regex LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(example_gdbm example_gdbm.cpp)
target_link_libraries (example_gdbm LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable( example_readstdout  example_readstdout.cpp)
target_link_libraries ( example_readstdout LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(example_sqlite example_sqlite.cpp)
target_link_libraries (example_sqlite LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable( example_strftime  example_strftime.cpp)
target_link_libraries ( example_strftime LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(example_findimageinfo example_findimageinfo.cpp)
target_link_libraries (example_findimageinfo LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable( example_iter  example_iter.cpp)
target_link_libraries ( example_iter LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(example_boost_options example_boost_options.cpp)
target_link_libraries (example_boost_options LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(example_netserver example_netserver.cpp)
target_link_libraries (example_netserver LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)



add_executable(fixedstringextractor fixedstringextractor.cpp)
target_link_libraries (fixedstringextractor LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)


add_executable(sqlextractor sqlextractor.cpp)
target_link_libraries (sqlextractor LINK_PUBLIC libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_custom_command( OUTPUT sqldefinitions.cpp sqldefinitions.h sqldefinitions.sql
  COMMAND sqlextractor ARGS ${CMAKE_CURRENT_SOURCE_DIR}/wikiobjects.h
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/wikiobjects.h
  DEPENDS sqlextractor )

add_custom_command( OUTPUT openlayerstemplate.cpp openlayerstemplate.h
  COMMAND fixedstringextractor ARGS sections ${CMAKE_CURRENT_SOURCE_DIR}/openlayerstemplate.js openlayerstemplate
  MAIN_DEPENDENCY openlayerstemplate.js
  DEPENDS fixedstringextractor )

add_library( fbywiki sqldefinitions.cpp wiki.cpp wikidb.cpp wikiobjects.cpp wikinode.cpp wikivideoflash.cpp wikistatusupdate.cpp  wikiopenlayers.cpp wikidpl.cpp openlayerstemplate.cpp )


add_executable(fby fby.cpp)
target_link_libraries (fby LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(fby_test fby_test.cpp)
target_link_libraries (fby_test LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(test_fbydb test_fbydb.cpp)
target_link_libraries (test_fbydb LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(scaninputwikifiles scaninputwikifiles.cpp)
target_link_libraries (scaninputwikifiles LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(scanimages scanimages.cpp)
target_link_libraries (scanimages LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(parsewikifiletostaging parsewikifiletostaging.cpp)
target_link_libraries (parsewikifiletostaging LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(test_regexparse test_regexparse.cpp)
target_link_libraries (test_regexparse LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(parsewikifilesforlinks parsewikifilesforlinks.cpp)
target_link_libraries (parsewikifilesforlinks LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(scanwikifilesforlinks scanwikifilesforlinks.cpp)
target_link_libraries (scanwikifilesforlinks LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(rebuilddirtyfiles rebuilddirtyfiles.cpp)
target_link_libraries (rebuilddirtyfiles LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(getstatus getstatus.cgi.cpp)
target_link_libraries (getstatus LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(setstatus setstatus.cgi.cpp)
target_link_libraries (setstatus LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(test_fbyregex test_fbyregex.cpp)
target_link_libraries (test_fbyregex LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(test_microformats test_microformats.cpp)
target_link_libraries (test_microformats LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(test_parsehtml test_parsehtml.cpp)
target_link_libraries (test_parsehtml LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

add_executable(test_nethttpheaders test_nethttpheaders.cpp)
target_link_libraries (test_nethttpheaders LINK_PUBLIC fbywiki libfby sqlite3 pq boost_filesystem boost_system gdbm pcre boost_iostreams boost_program_options cgicc /home/danlyke/lib/libgtest.a pthread)

