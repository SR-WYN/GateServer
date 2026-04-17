#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"
#include "RPConPool.h"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <memory>

VerifyGrpcClient::VerifyGrpcClient()
{
    auto& gCfgMgr = ConfigMgr::GetInstance();
    std::string host = gCfgMgr["VerifyServer"]["Host"];
    std::string port = gCfgMgr["VerifyServer"]["Port"];
    std::cout << "VerifyServer host is " << host << " port is " << port << std::endl;
    _pool.reset(new RPConPool(5,host,port));
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
    auto stub = _pool->GetConnection();
    Status status = stub->GetVerifyCode(&context, request, &reply);
    if (status.ok())
    {
        _pool->ReturnConnection(std::move(stub));
        std::cout << "GetVerifyCode success" << std::endl;
        return reply;
    }
    else
    {
        _pool->ReturnConnection(std::move(stub));
        reply.set_error(ErrorCodes::RPCFAILED);
        std::cout << "error is " << status.error_code() << " " << status.error_message()
                  << std::endl;
        return reply;
    }
}
