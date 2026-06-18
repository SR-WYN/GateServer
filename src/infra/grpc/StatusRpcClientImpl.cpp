// StatusRpcClientImpl.cpp - StatusRpcClient 适配器实现
#include "StatusRpcClientImpl.h"
#include "StatusGrpcClient.h"

GetChatServerRsp StatusRpcClientImpl::getChatServer(int uid)
{
    return StatusGrpcClient::getInstance().getChatServer(uid);
}
