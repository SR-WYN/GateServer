add_library(gs_common STATIC utils.cpp Log.cpp)
target_link_libraries(gs_common PUBLIC gateserver_includes spdlog::spdlog)
