add_executable(WebSocketServer WebSocketServer.cpp)
target_link_libraries(
    WebSocketServer
    src
    ${BOOST_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

