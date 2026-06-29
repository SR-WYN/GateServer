add_library(common STATIC
    utils/utils.cpp
    utils/json_util.cpp
)

target_link_libraries(common PUBLIC
    gateserver_includes
    JsonCpp::JsonCpp
    Boost::system
    spdlog::spdlog
)