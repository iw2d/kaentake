# Build injector
add_library(injector SHARED
    injector.cpp
    system.cpp
    bypass.cpp
    inlink.cpp
    resman.cpp
    avatar.cpp
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

add_custom_command(TARGET injector POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Custom.wz"
        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
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