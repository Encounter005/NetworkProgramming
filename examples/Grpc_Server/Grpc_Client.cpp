#include "../../build/proto/gen_cxx/demo.grpc.pb.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/security/server_credentials.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientWriter;
using grpc::Status;
using hello::Chunk;
using hello::Greeter;
using hello::HelloReply;
using hello::HelloRequest;
using hello::Reply;

constexpr int CHUNK_SIZE = 1024 * 1024;

class FCClient {
public:
    FCClient(std::shared_ptr<Channel> channel);
    std::string SayHello(const std::string &name);
    void        PathToUpload(const std::string &path);

private:
    void                           Upload(const std::string &filepath);
    std::unique_ptr<Greeter::Stub> stub_;
};


FCClient::FCClient(std::shared_ptr<Channel> channel)
    : stub_(Greeter::NewStub(channel)) {}

std::string FCClient::SayHello(const std::string &name) {
    ClientContext client_context;
    HelloReply    reply;
    HelloRequest  request;
    request.set_message(name);
    Status status = stub_->SayHello(&client_context, request, &reply);

    if (status.ok()) {
        return reply.message();
    } else {
        return "failure " + status.error_message();
    }
}

void FCClient::Upload(const std::string &filepath) {
    Chunk         chunk;
    char          data[CHUNK_SIZE];
    Reply         stats;
    ClientContext client_context;
    std::ifstream inputfile;
    int           len = 0;

    std::string::size_type pos = filepath.find_last_of('/') + 1;
    std::string filename       = filepath.substr(pos, filepath.length() - pos);


    inputfile.open(filepath, std::ifstream::in | std::ifstream::binary);

    if (inputfile.is_open()) {
        std::cout << "File: " << filename << " open success" << std::endl;
    } else {
        std::cout << "File: " << filename << " open failed" << std::endl;
        throw "File open failed";
    }

    std::unique_ptr<ClientWriter<Chunk>> writer(
        stub_->Upload(&client_context, &stats));

    while (!inputfile.eof()) {
        inputfile.read(data, CHUNK_SIZE);
        chunk.set_buffer(data, inputfile.gcount());
        chunk.set_filename(filename);
        if (!writer->Write(chunk)) {
            break;
        }
        len += inputfile.gcount();
    }

    writer->WritesDone();
    Status status = writer->Finish();
    if (status.ok()) {
        std::cout << "Transfer " << filename << " success, " << len << " bytes"
                  << std::endl;
    } else {
        std::cout << "Transfer " << filename << " failed, "
                  << status.error_message() << std::endl;
    }
    inputfile.close();
}


void FCClient::PathToUpload(const std::string &path) {
    boost::filesystem::path dir_path(path);

    if (!boost::filesystem::exists(dir_path)
        || !boost::filesystem::is_directory(dir_path)) {
        throw std::runtime_error(
            "Directory does not exist or is not a directory: " + path);
    }

    for (const auto &entry : boost::filesystem::directory_iterator(dir_path)) {
        this->Upload(entry.path().string());
    }
}


int main(int argc, char *argv[]) {
    std::string server_address("127.0.0.1:50001");
    auto        channel = grpc::CreateChannel(
        server_address, grpc::InsecureChannelCredentials());


    try {

        FCClient client(channel);
        client.SayHello("I will transfer files");

        std::string path = "./test_input_files";
        client.PathToUpload(path);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
