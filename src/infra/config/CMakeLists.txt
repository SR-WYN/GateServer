add_library(gs_config STATIC ConfigMgr.cpp)
target_link_libraries(gs_config PUBLIC gateserver_includes Boost::filesystem JsonCpp::JsonCpp spdlog::spdlog)
