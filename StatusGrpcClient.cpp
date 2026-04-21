#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "StatusConPool.h"
#include "const.h"
#include "utils.h"

StatusGrpcClient::StatusGrpcClient()
{
    auto& gCfgMgr = ConfigMgr::GetInstance();
    std::string host = gCfgMgr["StatusServer"]["Host"];
    std::string port = gCfgMgr["StatusServer"]["Port"];
    _pool.reset(new StatusConPool(5, host, port));
}

StatusGrpcClient::~StatusGrpcClient()
{
}

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
{
    ClientContext context;
    GetChatServerRsp reply;
    GetChatServerReq request;
    request.set_uid(uid);
    auto stub = _pool->GetConnection();
    Status status = stub->GetChatServer(&context, request, &reply);
    utils::Defer defer(
        [&stub, this]()
        {
            _pool->ReturnConnection(std::move(stub));
        });
    if (status.ok())
    {
        return reply;
    }
    else
    {
        reply.set_error(ErrorCodes::RPCFAILED);
        return reply;
    }
}