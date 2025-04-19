#include "../../build/proto/gen_cxx/demo.grpc.pb.h"
#include <fstream>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server_context.h>
#include <iostream>
#include <memory>
#include <string>


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
        ServerContext *context, const HelloRequest *request,
        HelloReply *reply) override {
        std::cout << "Connect a client from: " << context->peer() << " message: " << request->message() << std::endl;
        std::string prefix("Server received: ");
        reply->set_message(prefix + request->message());
        return Status::OK;
    }

    Status Upload(
        ServerContext *context, ServerReader<Chunk> *reader,
        hello::Reply *reply) override {
        Chunk         chunk;
        std::ofstream outputfile;
        const char   *data;
        bool          bRead = reader->Read(&chunk);
        if (!bRead) {
            reply->set_length(0);
            return Status::OK;
        }

        std::string filename = chunk.filename();
        std::string output_folder = "./test_output_files/";
        std::string output_path = output_folder + filename;


        // NOTE:
        //  - std::ostream::out
        //      是一个打开模式标志，表示以输出模式打开文件。
        //      文件将被打开以便写入数据。如果文件不存在，则会创建一个新文件。
        //      如果文件已经存在，文件内容将被清空。
        //  - std::ofstream::app
        //      这是一个打开模式标志，表示以追加模式打开文件。
        //      文件将被打开以便写入数据，但所有写入操作将在文件末尾进行，不会覆盖文件的现有内容。
        //  - std::ofstream::binary
        //      这是一个打开模式标志，表示以二进制模式打开文件。
        //      文件将被打开以便进行二进制数据读写操作，而不是文本数据读写操作。
        // 可以用管道符将多个打开模式组合在一起
        //      以输出模式打开文件 filename。
        //      以追加模式打开文件，所有写入操作将在文件末尾进行。
        //      以二进制模式打开文件，进行二进制数据读写操作。
        outputfile.open(
            output_path,
            std::ostream::out | std::ofstream::app | std::ofstream::binary);

        if (outputfile.is_open()) {
            std::cout << "File: " << filename << " open success" << std::endl;
        } else {
            std::cout << "File: " << filename << " open failed" << std::endl;
        }

        while (bRead && !(chunk.buffer()).empty()) {
            data = chunk.buffer().c_str();
            outputfile.write(data, chunk.buffer().length());
            bRead = reader->Read(&chunk);
        }

        int64_t pos = outputfile.tellp();
        reply->set_length(static_cast<int32_t>(pos));
        outputfile.close();
        return Status::OK;
    }
};

void RunServer() {
    std::string        server_address("127.0.0.1:50001");
    GreeterServiceImpl service;
    ServerBuilder      builder;

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
