add_library(texc_keyhook
    keyhook.c
)

target_link_libraries(texc_keyhook texc_match texc_expand)
