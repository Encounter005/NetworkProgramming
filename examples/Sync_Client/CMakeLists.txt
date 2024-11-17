add_executable(Sync_Client Sync_Client.cpp)

target_link_libraries(
    Sync_Client
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)
