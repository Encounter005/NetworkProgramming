add_executable(HttpServer HttpServer.cpp)
add_executable(HttpClient HttpClient.cpp)

target_link_libraries(
    HttpServer
    src
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

target_link_libraries(
    HttpClient
    src
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

