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
#include <stdexcept>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "fib.grpc.pb.h"


#define MAX_FIB_NUM             93      // More than 93 will not fit into uint64
#define FIB_PRECOMP_RANGE       80      // Precompute the Fibonacci values so that we get smaller latency.
                                        // This value is smaller than MAX_FIB_NUM to test both the precomputation
                                        // code and the calculation based on precomputed values



class FibServiceImpl final : public fib::FibService::Service {
public:
    FibServiceImpl() : _fibNumbers(FIB_PRECOMP_RANGE + 1) {

        // Precompute the Fibonacci sequence
        _fibNumbers[0] = 0;
        _fibNumbers[1] = 1;
        _fibNumbers[2] = 1;

        for(uint32_t i = 3; i <= FIB_PRECOMP_RANGE; ++i)
            _fibNumbers[i] = _fibNumbers[i - 2] + _fibNumbers[i - 1];
    }

    grpc::Status ComputeFib(grpc::ServerContext* ctx, const fib::FibRequest* req, fib::FibResponse* rep) override {

        int32_t requested_fib = 0;
        
        // Requested Fibonacci number should be in range [0,93] so that the result fits into uint64
        try {
            requested_fib = stoi(req->n());
        }
        catch(const std::out_of_range& oor) {
            return grpc::Status(grpc::StatusCode::OUT_OF_RANGE, "Requested Fibonacci number is out of range [0, 93]");
        }
        catch(const std::invalid_argument& ie) {
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Requested Fibonacci number cannot be parsed");
        }
        
        if(requested_fib < 0 || requested_fib > MAX_FIB_NUM)
            return grpc::Status(grpc::StatusCode::OUT_OF_RANGE, "Requested Fibonacci number is out of range [0, 93]");


        std::cout << "Responding to request :: n = " << requested_fib << std::endl;
        
        rep->set_fib(CalcFibHelper(requested_fib));
        rep->set_count(++_counts[requested_fib]);
        rep->set_timestamp((uint32_t)time(nullptr));

        return grpc::Status::OK;
    }

private:
    uint64_t CalcFibHelper(int32_t n) {

        // If MAX_FIB_NUM <= FIB_PRECOMP_RANGE we already have precomputed the result, however,
        // the rest of the function below is for cases when we decide that FIB_PRECOMP_RANGE < MAX_FIB_NUM
        if(n <= FIB_PRECOMP_RANGE)
            return _fibNumbers[n];
        
        int32_t i = FIB_PRECOMP_RANGE;
        uint64_t f1 = _fibNumbers[FIB_PRECOMP_RANGE - 1];
        uint64_t f2 = _fibNumbers[FIB_PRECOMP_RANGE];
        for( ; i < n; ++i) {
            uint64_t tmp = f2;
            f2 = f1 + f2;
            f1 = tmp;
        }
        return f2;
    }

    std::vector<uint64_t> _fibNumbers;
    std::unordered_map<int32_t, uint32_t> _counts;
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