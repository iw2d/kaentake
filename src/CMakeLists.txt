# Build injector
add_library(injector SHARED
    injector.cpp
    system.cpp
    bypass.cpp
    resman.cpp
    resolution.cpp
    debug.cpp
    hook.cpp
)

target_precompile_headers(injector PRIVATE pch.h)

target_link_libraries(injector PRIVATE
    Detours
    WzLib
)

set_target_properties(injector PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
)


# Build launcher
add_executable(launcher WIN32
    launcher.manifest
    launcher.rc
    launcher.cpp
    debug.cpp
)

target_link_options(launcher PRIVATE "/MANIFEST:NO")

target_link_libraries(launcher PRIVATE
    Detours
)

set_target_properties(launcher PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
)