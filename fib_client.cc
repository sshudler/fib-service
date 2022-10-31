
#include <iostream>
#include <string>
#include <cstdint>

#include <grpcpp/grpcpp.h>

#include "fib.grpc.pb.h"


class FibClient {
public:
    FibClient(std::shared_ptr<grpc::Channel> channel) : _stub(fib::FibService::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server
    bool ComputeFib(int32_t n, uint64_t& fib, uint32_t& ts, uint32_t& count) {

        fib::FibRequest req;
        req.set_n(std::to_string(n));

        // Container for the data we expect from the server
        fib::FibResponse rep;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors
        grpc::ClientContext ctx;

        // The actual RPC
        grpc::Status status = _stub->ComputeFib(&ctx, req, &rep);

        // Act upon its status
        if(status.ok()) {
            fib = rep.fib();
            ts = rep.timestamp();
            count = rep.count();
        }
        else {
           std::cout << status.error_code() << ": " << status.error_message() << std::endl;
        }

        return status.ok();
    }

private:
    std::unique_ptr<fib::FibService::Stub> _stub;
};


int main(int argc, char** argv) {

    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint which is localhost:9090
    // by default. We indicate that the channel isn't authenticated (use of 
    // InsecureChannelCredentials()).

    std::string target_str = "localhost:9090";

    if(argc < 2) {
        std::cout << "Usage: fib_client <fib number>" << std::endl;
        return -1;
    }

    int32_t n = std::stoi(argv[1]);

    FibClient fib_service(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

    uint32_t ts, cnt;
    uint64_t f;
    if(!fib_service.ComputeFib(n, f, ts, cnt)) {
        std::cout << "Request failed" << std::endl;
        return -2;
    }
    std::cout << "Fibonacci of " << n << ":" << std::endl;
    std::cout << "fib = " << f << std::endl;
    std::cout << "ts = " << ts << std::endl;
    std::cout << "count = " << cnt << std::endl;

    return 0;
}