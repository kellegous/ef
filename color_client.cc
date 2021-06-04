#include "color_client.h"

grpc::Status ColorClient::GetRandomTheme(
    pkg::Theme* theme,
    uint64_t seed
) const {
    rpc::RandReq req;
    req.set_n(1);
    req.set_seed(seed);

    rpc::RandRes res;

    grpc::ClientContext context;

    grpc::Status status = stub_->GetRandom(
        &context,
        req,
        &res);
    if (!status.ok()) {
        return status;
    }

    theme->CopyFrom(res.themes().Get(0));
    return status;
}

std::unique_ptr<ColorClient> ColorClient::Create(
    const std::string& addr
) {
    return std::unique_ptr<ColorClient>(
        new ColorClient(grpc::CreateChannel(addr, grpc::InsecureChannelCredentials()))
    );
}