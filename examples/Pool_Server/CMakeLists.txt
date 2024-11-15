add_executable(IOServicePoolServer IOServicePoolServer.cpp)
add_executable(IOThreadPoolServer IOThreadPoolServer.cpp)

target_link_libraries(
    IOServicePoolServer
    src
    ${BOOST_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)
target_link_libraries(
    IOThreadPoolServer
    src
    ${BOOST_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

