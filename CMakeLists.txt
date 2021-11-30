cmake_minimum_required(VERSION 3.16)
include(GNUInstallDirs)

set(SVERSION 1.00)
set(APP_NAME "uSecureTAP")
project(${APP_NAME})
project(${PROJECT_NAME} VERSION ${SVERSION} DESCRIPTION "Unmanarc Secure TAP")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

file(GLOB_RECURSE EDV_INCLUDE_FILES "src/*.h*")
file(GLOB_RECURSE EDV_SOURCE_FILES "src/*.c*")

file(GLOB_RECURSE EDV_INCLUDE_FILES2 "src/*/*.h*")
file(GLOB_RECURSE EDV_SOURCE_FILES2 "src/*/*.c*")

add_executable( ${APP_NAME} ${EDV_INCLUDE_FILES} ${EDV_SOURCE_FILES} ${EDV_INCLUDE_FILES2} ${EDV_SOURCE_FILES2} )

install( TARGETS ${APP_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )

if (EXTRAPREFIX)
    target_include_directories(${APP_NAME} PUBLIC ${EXTRAPREFIX}/include)
    target_link_libraries(${APP_NAME} "-L${EXTRAPREFIX}/lib")
    target_link_libraries(${APP_NAME} "-L${EXTRAPREFIX}/lib64")
endif()

target_link_libraries(${APP_NAME} "-lcx2_mem_vars")
target_link_libraries(${APP_NAME} "-lcx2_prg_service")
target_link_libraries(${APP_NAME} "-lcx2_net_interfaces")
target_link_libraries(${APP_NAME} "-lcx2_prg_logs")
target_link_libraries(${APP_NAME} "-lcx2_net_sockets")
target_link_libraries(${APP_NAME} "-lcx2_thr_mutex")
target_link_libraries(${APP_NAME} "-lcx2_thr_threads")
target_link_libraries(${APP_NAME} "-lcx2_hlp_functions")
target_link_libraries(${APP_NAME} "-lcx2_thr_safecontainers")
target_link_libraries(${APP_NAME} "-lpthread")
target_link_libraries(${APP_NAME} "-ljsoncpp")
target_link_libraries(${APP_NAME} "-lssl")
target_link_libraries(${APP_NAME} "-lcrypto")

if (WIN32)
    target_link_libraries(${APP_NAME} "-lws2_32")
elseif()
    target_link_libraries(${APP_NAME} "-lrt -ldl")
endif()

######################################

find_library (
    BOOST_REGEX_LIB
    NAMES boost_regex-mt-x32 libboost_regex-mt-x32 boost_regex-mt libboost_regex-mt boost_regex libboost_regex # what to look for
)
if (NOT BOOST_REGEX_LIB)
    message("ERROR: Did not found lib BOOST_REGEX_LIB")
else()
    message("-- BOOST_REGEX_LIB found at [${BOOST_REGEX_LIB}] for ${APP_NAME}")
    target_link_libraries(${APP_NAME} ${BOOST_REGEX_LIB})
endif()

