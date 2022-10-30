//
// A server implementation for the Fibonacci service, based
// on: https://github.com/grpc/grpc/blob/master/examples/cpp/helloworld/greeter_server.cc
//

#include <iostream>
#include <string>
#include <cstdint>
#include <ctime>
#include <vector>
#include <unordered_map>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "fib.grpc.pb.h"


#define FIB_PRECOMP_RANGE       1048576     // 2^20 = 1M


class FibServiceImpl final : public fib::FibService::Service {
public:
    FibServiceImpl() : _fibNumbers(FIB_PRECOMP_RANGE) {
        _fibNumbers[0] = 0;
        _fibNumbers[1] = 1;
        _fibNumbers[2] = 1;

        for(uint32_t i = 3; i < FIB_PRECOMP_RANGE; ++i)
            _fibNumbers[i] = _fibNumbers[i - 2] + _fibNumbers[i - 1];
    }

    grpc::Status ComputeFib(grpc::ServerContext* ctx, const fib::FibRequest* req, fib::FibResponse* rep) override {

        std::cout << "Responding to request -- n = " << req->n() << std::endl;

        uint32_t requested_fib = req->n();
        rep->set_fib(CalcFibHelper(requested_fib));
        rep->set_count(++_counts[requested_fib]);
        rep->set_timestamp((uint32_t)time(nullptr));

        return grpc::Status::OK;
    }

private:
    uint32_t CalcFibHelper(uint32_t n) {

        if(n < FIB_PRECOMP_RANGE)
            return _fibNumbers[n];
        
        uint32_t i = FIB_PRECOMP_RANGE;
        uint32_t f2 = _fibNumbers[FIB_PRECOMP_RANGE - 2];
        uint32_t f1 = _fibNumbers[FIB_PRECOMP_RANGE - 1];
        for( ; i < n - 1; ++i) {
            uint32_t tmp = f2;
            f2 = f1 + f2;
            f1 = f2;
        }
        return f2;
    }

    std::vector<uint32_t> _fibNumbers;
    std::unordered_map<uint32_t, uint32_t> _counts;
};


void RunServer() {

    std::string server_address("localhost:9090");

    FibServiceImpl service_impl;
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;

    // Listen on the given address without any authentication mechanism
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service
    builder.RegisterService(&service_impl);

    // Finally assemble the server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Fibonacci server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

int main(int argc, char** argv) {

    RunServer();

    return 0;
}