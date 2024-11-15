add_executable(Sync_Client Sync_Client.cpp)

target_link_libraries(
    Sync_Client
    proto_gen_cxx
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)
