#include "../../out/Debug/proto/gen_cxx/demo.grpc.pb.h"
#include <fstream>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_builder.h>
#include <iostream>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::Status;
using hello::Chunk;
using hello::Greeter;
using hello::HelloReply;
using hello::HelloRequest;

class GreeterServiceImpl final : public Greeter::Service {
    Status SayHello(
        ServerContext* context, const HelloRequest* request,
        HelloReply* reply) override {
        std::string prefix("grpc server has received: ");
        reply->set_message(prefix + request->message());
        return Status::OK;
    }

    Status Upload(
        ServerContext* context, ServerReader<::hello::Chunk>* reader,
        ::hello::Reply* reply) override {
        Chunk         chunk;
        std::ofstream outfile;
        const char*   data;
        bool          bRead = reader->Read(&chunk);
        if (!bRead) {
            reply->set_length(0);
            return Status::OK;
        }

        std::string filename = chunk.filename();
        outfile.open(
            filename,
            std::ofstream::out | std::ofstream::app | std::ofstream::binary);

        if (outfile.is_open()) {
            std::cout << "文件已经成功打开!" << std::endl;
        } else {
            std::cout << "无法打开文件" << std::endl;
        }

        while (bRead && !(chunk.buffer().empty())) {
            data = chunk.buffer().c_str();
            outfile.write(data, chunk.buffer().length());
            bRead = reader->Read(&chunk);
        }

        long pos = outfile.tellp();
        reply->set_length(pos);
        outfile.close();

        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("127.0.0.1:50051");
    GreeterServiceImpl service;
    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}
