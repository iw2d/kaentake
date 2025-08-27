add_library(injector SHARED
    injector.cpp
    system.cpp
    bypass.cpp
    resman.cpp
    debug.cpp
    hook.cpp
)

target_link_libraries(injector PRIVATE
    Detours
    Gr2D_DX8to9
    WzLib
)

set_target_properties(injector PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
)