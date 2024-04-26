add_library(texc_http
    server.c
    server_api.c

    client.c
)

target_link_libraries(texc_http texc_data)
if(WIN32)
    target_link_libraries(texc_http wsock32 ws2_32)
endif()
