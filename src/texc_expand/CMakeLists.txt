add_library(texc_expand
    keyboard.c
    expand.c
)

if (NOT WIN32)
    target_link_libraries(texc_expand X11 Xtst)
endif()
