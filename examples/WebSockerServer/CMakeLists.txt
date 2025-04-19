add_executable(WebSocketServer WebSocketServer.cpp)
target_link_libraries(
    WebSocketServer
    include
    src
    ${BOOST_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

