add_executable(grpcServer Grpc_Server.cpp)
add_executable(grpcClient Grpc_Client.cpp)
add_executable(FileTransfer FileTransfer.cpp)

target_link_libraries(
    grpcServer
    proto_gen
    ${PROTOBUF_LIBRARIES}
    ${_GRPC_GRPCPP}
)

target_link_libraries(
    grpcClient
    proto_gen
    ${PROTOBUF_LIBRARIES}
    ${_GRPC_GRPCPP}
    ${Boost_LIBRARIES}
)

target_link_libraries(
    FileTransfer
    proto_gen
    ${PROTOBUF_LIBRARIES}
    ${_GRPC_GRPCPP}
)

