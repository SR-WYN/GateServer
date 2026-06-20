add_library(gs_base STATIC
    Singleton.h
    ThreadPool.h
    ThreadPool.cpp
    ThreadPoolMgr.h
    ThreadPoolMgr.cpp
    utils.cpp
)

target_link_libraries(gs_base PUBLIC gateserver_includes)
