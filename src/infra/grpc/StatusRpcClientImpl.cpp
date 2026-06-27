// StatusRpcClientImpl.cpp - StatusRpcClient 适配器实现
#include "StatusRpcClientImpl.h"
#include "StatusGrpcClient.h"

GetChatServerRsp StatusRpcClientImpl::getChatServer(int uid)
{
    return StatusGrpcClient::getInstance().getChatServer(uid);
}

int StatusRpcClientImpl::validateToken(int uid, const std::string& token)
{
    return StatusGrpcClient::getInstance().validateToken(uid, token);
}
