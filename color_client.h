#ifndef _COLOR_CLIENT_H_
#define _COLOR_CLIENT_H_

#include "pkg/rpc/rpc.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

class ColorClient {
public:
    ColorClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(rpc::Colors::NewStub(channel)) {}

    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ColorClient);

    grpc::Status GetRandomTheme(pkg::Theme* theme, uint64_t seed) const;

    static std::unique_ptr<ColorClient> Create(const std::string& addr);
private:
    std::unique_ptr<rpc::Colors::Stub> stub_;
};

#endif // _COLOR_CLIENT_H_