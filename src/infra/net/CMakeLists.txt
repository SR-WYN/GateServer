add_library(gs_net STATIC
    CServer.cpp
    HttpConnection.cpp
)

target_link_libraries(gs_net PUBLIC
    gateserver_includes
    Boost::system
    Boost::thread
)
