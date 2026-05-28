# GateServer 日志使用说明

## 1. 日志写到哪里

`config.json` 中 `Log.Dir` 为日志目录的**绝对路径**。所有 `.log` 直接放在该目录下，例如：

`/home/asus/NETLEARN/GateServer/src/logdir/app.log`

| 枚举 `LogModule` | 文件名 | 用途 |
|------------------|--------|------|
| `App` | `app.log` | 进程启动、退出、异常（`app/GateServer.cpp`） |
| `Config` | `config.log` | 配置加载（`infra/config/ConfigMgr`） |
| `Http` | `http.log` | HTTP 监听、连接读写出错（`net/HttpConnection`、`CServer`） |
| `Mysql` | `mysql.log` | 用户注册、登录校验、改密等数据库访问（`infra/mysql/`） |
| `Redis` | `redis.log` | 验证码、会话等缓存（`infra/redis/`） |
| `Grpc` | `grpc.log` | 验证码服务、Status 等 gRPC 调用（`infra/grpc/`） |
| `Logic` | `logic.log` | 注册、登录、找回密码、拉取 Chat 节点等 HTTP 业务（`application/LogicSystem`） |

只有**实际打过日志**的模块才会生成对应文件。

---

## 2. 配置 `config.json`

```json
"Log": {
  "Dir": "/home/asus/NETLEARN/GateServer/src/logdir",
  "Level": "info"
}
```

`Level`：`trace` / `debug` / `info` / `warn` / `error` / `critical` / `off`。

---

## 3. 初始化（`main` 里，`ConfigMgr` / `init()` 之后）

```cpp
#include "ConfigMgr.h"
#include "Log.h"

ConfigMgr::getInstance();
init();  // 已有逻辑

if (!Log::init("GateServer", ConfigMgr::getInstance().getLogConfig())) {
    return EXIT_FAILURE;
}

// 退出前
Log::shutdown();
```

---

## 4. 在业务代码里打日志

```cpp
#include "Log.h"

Log::info(LogModule::Http, "POST {}", path);
Log::error(LogModule::Logic, "register failed: {}", reason);

LOGI(LogModule::Mysql, "new user uid={}", uid);
```

### 4.1 级别

`Log::trace/debug/info/warn/error/critical`，宏 `LOGT` `LOGD` `LOGI` `LOGW` `LOGE` `LOGC`。

### 4.2 格式

消息使用 `{}` 占位，例如：`Log::warn(LogModule::Redis, "AUTH failed");`

### 4.3 新增模块

新增模块：改 [`LogModule.h`](LogModule.h) 内 `LogModule` 枚举、`LogNames::_xxx` 字符串常量与 `_table`（顺序一致），不要用字符串模块名。

命名约定：成员变量 `_snake_case`（如 `LogConfig::_dir`、`Log::_mutex`）；成员函数小写驼峰（如 `init`、`createModuleLogger`、`moduleName`）。

## 文件说明

`LogModule.h`、`Log.h`、`Log.cpp` 三个源文件；配置见 `ConfigMgr::getLogConfig()`。

---

## 5. 按目录选哪个模块（速查）

| 代码位置 | 使用 |
|----------|------|
| `src/app/` | `LogModule::App` |
| `src/infra/config/` | `LogModule::Config` |
| `src/net/` | `LogModule::Http` |
| `src/application/LogicSystem.*` | `LogModule::Logic` |
| `src/infra/mysql/` | `LogModule::Mysql` |
| `src/infra/redis/` | `LogModule::Redis` |
| `src/infra/grpc/` | `LogModule::Grpc` |

---

## 6. 注意

- `Log::init` 成功后再写日志。
- 工作目录下需有 `config.json`。
- 仅写文件，无控制台输出；无按大小轮转。
