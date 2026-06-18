add_library(gs_log STATIC Log.cpp)
target_link_libraries(gs_log PUBLIC gateserver_includes spdlog::spdlog)
