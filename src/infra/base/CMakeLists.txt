add_library(gs_base STATIC
    Singleton.h
    utils.cpp
)

target_link_libraries(gs_base PUBLIC gateserver_includes)
