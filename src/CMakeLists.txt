# SPDX-License-Identifier: Unlicense

target_sources( ${PROJECT_NAME}
    PRIVATE
        brtl.h
        brtl.cpp
        noctis-d.h
        additional_math.h
        noctis-0.h
        noctis-0.cpp
        noctis-1.cpp
        noctis.h
        noctis.cpp
        noctis-f.cpp
        Feltyrion.h
        Feltyrion.cpp
        GDExtensionTemplate.h
        GDExtensionTemplate.cpp
        RegisterExtension.cpp
)

target_compile_definitions( ${PROJECT_NAME} PRIVATE WITH_GODOT)

target_include_directories( ${PROJECT_NAME}
    PRIVATE
        "src"
)
