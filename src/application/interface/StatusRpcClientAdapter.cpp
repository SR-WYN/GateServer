// StatusRpcClientAdapter.cpp - IStatusRpcClient 适配器实现
#include "StatusRpcClientAdapter.h"
#include "StatusGrpcClient.h"

GetChatServerRsp StatusRpcClientAdapter::getChatServer(int uid)
{
    return StatusGrpcClient::getInstance().getChatServer(uid);
}
