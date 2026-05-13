# 入口在 app/GateServer.cpp，由上层 add_executable 加入
add_library(gateserver_app_dir INTERFACE)
target_link_libraries(gateserver_app_dir INTERFACE gateserver_includes)
