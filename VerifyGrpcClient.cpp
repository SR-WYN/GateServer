#include "VerifyGrpcClient.h"
#include "message.pb.h"

VerifyGrpcClient::VerifyGrpcClient()
{
    std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
    _stub = VerifyService::NewStub(channel);
}

VerifyGrpcClient::~VerifyGrpcClient()
{
}

GetVerifyRsp VerifyGrpcClient::GetVerifyCode(std::string email)
{
    ClientContext context;
    GetVerifyRsp reply;
    GetVerifyReq request;
    request.set_email(email);
    Status status = _stub->GetVerifyCode(&context, request, &reply);
    if (status.ok())
    {
        std::cout << "GetVerifyCode success" << std::endl;
        return reply;
    }
    else 
    {
        reply.set_error(ErrorCodes::RPCFAILED);
        std::cout << "error is " << status.error_code() << " " << status.error_message() << std::endl;
        return reply;
    }
}