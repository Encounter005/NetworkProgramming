#include "../../out/Debug/proto/gen_cxx/demo.grpc.pb.h"
#include <grpcpp/completion_queue.h>
#include <grpc++/grpc++.h>
#include <iostream>
#include <memory>
#include <string>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using hello::Greeter;
using hello::HelloReply;
using hello::HelloRequest;

class GreeterServiceImpl final : public Greeter::Service {
    Status SayHello(
        ServerContext *context, const HelloRequest *request,
        HelloReply *reply) override {
        std::cout << "Request SayHello from " << context->peer() << std::endl;
        std::string prefix("Server received: ");
        reply->set_message(prefix + request->message());
        return Status::OK;
    }

    Status SayHelloAgain(
        ServerContext *context, const HelloRequest *request,
        HelloReply *reply) override {
        std::cout << "Request SayHelloAgain from " << context->peer()
                  << std::endl;
        std::string prefix("Server: Hey you sucker");
        reply->set_message(prefix);
        return Status::OK;
    }
};

void RunServer() {
    std::string        server_address("127.0.0.1:50001");
    GreeterServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}

int main(int argc, char *argv[]) {
    RunServer();
    return 0;
}
