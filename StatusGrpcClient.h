#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>
#include "Singleton.h"

using grpc::ClientContext;
using grpc::Status;

using message::GetChatServerRsp;
using message::GetChatServerReq;
using message::StatusService;

class StatusConPool;

class StatusGrpcClient : public Singleton<StatusGrpcClient>
{
    friend class Singleton<StatusGrpcClient>;
public:
    ~StatusGrpcClient() override;
    GetChatServerRsp getChatServer(int uid);
private:
    StatusGrpcClient();
    std::unique_ptr<StatusConPool> _pool;
};