# Build Detours
add_library(Detours STATIC
        Detours/src/creatwth.cpp
        Detours/src/modules.cpp
        Detours/src/detours.cpp
        Detours/src/disolx86.cpp
        Detours/src/disasm.cpp
        Detours/src/image.cpp
)
target_include_directories(Detours PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Detours/src)

# Build Gr2D_DX8to9
add_subdirectory(Gr2D_DX8to9)

# Build WzLib
set(WZLIB_NAME "WzLib" CACHE STRING "" FORCE)
add_subdirectory(WzLib)