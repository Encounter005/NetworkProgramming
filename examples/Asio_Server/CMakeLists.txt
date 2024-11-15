add_executable(AsioServer asio_server.cpp)
target_link_libraries(
    AsioServer
    src
    proto_gen
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)
