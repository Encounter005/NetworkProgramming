add_executable(grpcServer Grpc_Server.cpp)
add_executable(grpcClient Grpc_Client.cpp)

target_link_libraries(
    grpcServer
    src
    proto_gen
    ${PROTOBUF_LIBRARIES}
    ${GRPC_LIBRARIES}
)

target_link_libraries(
    grpcClient
    src
    proto_gen
    ${PROTOBUF_LIBRARIES}
    ${GRPC_LIBRARIES}
)

