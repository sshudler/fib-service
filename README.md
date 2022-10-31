# Fibonacci Web Service

A simple web service with a C++ backend to compute Fibonacci numbers. The client sends the requested index `n` and the server responds with the `n`'s Fibonacci number, a timestamp (i.e., number of seconds since 00:00 UTC, Jan 1, 1970), and a count of how many times `n` was already requested. For example, requesting `n = 20` will result in `{"fib":6765,"timestamp":1667133128,"count":1}` (JSON format).

Note that the index `n` is assumed to be in the range [0, 93], so that the resulting Fibonacci number fits into uint64. Otherwise the gRPC server returns an error status. In such case, Envoy will not produce any response.

The server uses the gRPC framework. It is based on Google's Protocol Buffers serialization infrastructure and communicates in the binary HTTP/2 format. To make it a web service we need HTTP/2 <--> HTTP/1.1 transcoding, and for this purpose the Envoy proxy is used.


## Build and run the service

1. Build and install gRPC and Protocol Buffers (https://grpc.io/docs/languages/cpp/quickstart/):

```bash
# It's important to set MY_INSTALL_DIR as Fibonacci server build script relies on it
export MY_INSTALL_DIR=$HOME/install
mkdir -p $MY_INSTALL_DIR
export PATH="$MY_INSTALL_DIR/bin:$PATH"

# Get a newer cmake
wget -q -O cmake-linux.sh https://github.com/Kitware/CMake/releases/download/v3.19.6/cmake-3.19.6-Linux-x86_64.sh
sh cmake-linux.sh -- --skip-license --prefix=$MY_INSTALL_DIR
rm cmake-linux.sh

# Install the basic tools required to build gRPC
sudo apt install -y build-essential autoconf libtool pkg-config

# Make sure git is recent enough to support --shallow-submodules
git clone --recurse-submodules -b v1.50.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc

# Build and install gRPC and Protocol Buffers
cd grpc
mkdir -p cmake/build
pushd cmake/build
cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR ../..
make -j
make install
popd
```

2. Install Envoy: https://www.envoyproxy.io/docs/envoy/latest/start/install. It can also run as a Docker container, but the server here was tested with Envoy running natively.

3. Clone the Google API repo and set `GOOGLEAPIS_DIR`:

```bash
git clone https://github.com/googleapis/googleapis
# It's important to set GOOGLEAPIS_DIR as Fibonacci server build script relies on it
export GOOGLEAPIS_DIR=<your-local-googleapis-folder>
```

4. Clone, build, and run the backend Fibonacci server and client. At this point the client can be used to connect to the server without the REST API:

```bash
git clone https://github.com/sshudler/fib-service.git
cd fib-service
mkdir _BUILD ; cd _BUILD
cmake ..
make -j
./fib_server
# This directory also contains the ./fib_client executable
```

5. Start Envoy using `envoy.yaml` configuration file:

```bash
# cd into where fib-service was cloned
envoy -c envoy.yaml
```

6. Test the setup by opening the browser and navigating to: http://localhost:8080/fib?n=20. It should return: `{"fib":6765,"timestamp":1667133128,"count":1}` as JSON data. The count will initially be `1` and further increasing for subsequent requests for the same Fibonacci number.


## Resources

Envoy:
- https://www.envoyproxy.io/docs/envoy/latest/start/install

gRPC:
- https://grpc.io/docs/what-is-grpc/introduction/
- https://grpc.io/docs/platforms/web/basics/
- https://grpc.io/docs/languages/cpp/basics/
- https://grpc.io/blog/coreos/

Examples:
- https://github.com/grpc/grpc-web/tree/master/net/grpc/gateway/examples/helloworld
- https://levelup.gitconnected.com/how-to-build-a-rest-api-with-grpc-and-get-the-best-of-two-worlds-9a4e491f30ae
- https://www.envoyproxy.io/docs/envoy/latest/configuration/http/http_filters/grpc_json_transcoder_filter
- https://levelup.gitconnected.com/how-to-build-a-rest-api-with-grpc-and-get-the-best-of-two-worlds-9a4e491f30ae


