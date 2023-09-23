cmake_minimum_required(VERSION 3.12)
project(NJEMU)

# Configuration

# Different target availables
set(TARGET_VALUES 
    "CPS1"  # 0
    "CPS2"  # 1
    "MVS"   # 2
    "NCDZ"  # 3
)
list(FIND TARGET_VALUES ${TARGET} TARGET_INDEX)
if (TARGET_INDEX EQUAL -1)
    message(FATAL_ERROR "Invalid TARGET. Please choose from the allowed values: ${TARGET_VALUES}")
endif()

# Add target to compiler definitions
add_definitions(-DBUILD_${TARGET})

# Different platform availables
set(PLATFORM_VALUES 
    "PSP"    # 0
    "PS2"    # 1 
    "X86_64" # 2
)
list(FIND PLATFORM_VALUES ${PLATFORM} PLATFORM_INDEX)
if (PLATFORM_INDEX EQUAL -1)
    message(FATAL_ERROR "Invalid PLATFORM. Please choose from the allowed values: ${PLATFORM_VALUES}")
endif()

# Get platform name lower case, used to include platform specific files
string(TOLOWER ${PLATFORM} PLATFORM_LOWER)

# Add platform to compiler definitions
add_definitions(-D${PLATFORM})

# Different options availables
option(LARGE_MEMORY "Large Memory" OFF)
option(KERNEL_MODE "Kernel Mode" OFF)
option(COMMAND_LIST "Command List" OFF)
option(ADHOC "Ad Hoc" OFF)
option(NO_GUI "No GUI" ON)
option(SAVE_STATE "Save State" OFF)
option(UI_32BPP "UI 32BPP" ON)
option(RELEASE "Release" OFF)
option(SYSTEM_BUTTONS "System Buttons" OFF)
option(DEBUG "Debug" ON)


# Version
set(VERSION_MAJOR 2)
set(VERSION_MINOR 4)
set(VERSION_BUILD 0)
add_definitions(-DVERSION_MAJOR=${VERSION_MAJOR})
add_definitions(-DVERSION_MINOR=${VERSION_MINOR})
add_definitions(-DVERSION_BUILD=${VERSION_BUILD})

# Define the version string
if (VERSION_BUILD EQUAL 0)
    set(VERSION_STR "${VERSION_MAJOR}.${VERSION_MINOR}")
else()
    set(VERSION_STR "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}")
endif()
add_definitions(-DVERSION_STR="${VERSION_STR}")

# Set compiler flags

# Common Warning options
set(WARNING_OPTIONS
    -Werror
    -Wno-unused-but-set-variable
    -Wno-unused-function
)

# Disable some warnigns based in platform
if (PLATFORM STREQUAL "PSP")
    list(APPEND WARNING_OPTIONS
        -Wno-unused-variable
    )
elseif (PLATFORM STREQUAL "PS2")
    list(APPEND WARNING_OPTIONS
        -Wno-unused-variable
        -Wno-format-overflow
        -Wno-unknown-pragmas
        -Wno-restrict
        -Wno-strict-aliasing
        -Wno-format
    )
elseif (PLATFORM STREQUAL "X86_64")
    list(APPEND WARNING_OPTIONS
        -Wno-unused-command-line-argument
        -Wno-invalid-source-encoding
    )
endif()

# Include directories
include_directories(
    src
    src/zip
    ${PLATFORM_LOWER}
)

# Common source files
set(COMMON_SRC
    emumain.c
    zip/zfile.c
    zip/unzip.c
    sound/sndintrf.c
    common/cache.c
    common/loadrom.c
    common/thread_driver.c
    common/audio_driver.c
    common/power_driver.c
    common/ticker_driver.c
    common/input_driver.c
    common/video_driver.c
    common/ui_text_driver.c
    common/platform_driver.c
    common/sound.c
)

# Additional source files based on options
if (ADHOC)
    set(COMMON_SRC ${COMMON_SRC}
        common/adhoc.c
    )
endif()

if (COMMAND_LIST)
    set(COMMON_SRC ${COMMON_SRC}
        common/cmdlist.c
    )
endif()

if (SAVE_STATE)
    set(COMMON_SRC ${COMMON_SRC}
        common/state.c
    )
endif()

if (NO_GUI)
    set(OS_SRC ${OS_SRC}
        ${PLATFORM_LOWER}/${PLATFORM_LOWER}_no_gui.c
    )
else()
    # Add source files related to GUI if needed
endif()


set(OS_SRC ${OS_SRC}
    ${PLATFORM_LOWER}/${PLATFORM_LOWER}_platform.c
    ${PLATFORM_LOWER}/${PLATFORM_LOWER}_ticker.c
    ${PLATFORM_LOWER}/${PLATFORM_LOWER}_power.c
    ${PLATFORM_LOWER}/${PLATFORM_LOWER}_ui_text.c
    ${PLATFORM_LOWER}/${PLATFORM_LOWER}_input.c
    ${PLATFORM_LOWER}/${PLATFORM_LOWER}_video.c
    ${PLATFORM_LOWER}/${PLATFORM_LOWER}_audio.c
    ${PLATFORM_LOWER}/${PLATFORM_LOWER}_thread.c
)

# Add specific target source files
if (${TARGET} STREQUAL "MVS")
    include_directories(
        src/cpu/m68000
        src/cpu/z80
        src/mvs
    )
    set(TARGET_SRC ${TARGET_SRC}
        cpu/m68000/m68000.c
        cpu/m68000/c68k.c
        cpu/z80/z80.c
        cpu/z80/cz80.c
    )

    set(TARGET_SRC ${TARGET_SRC}
        mvs/mvs.c
        mvs/driver.c
        mvs/memintrf.c
        mvs/inptport.c
        mvs/dipsw.c
        mvs/timer.c
        mvs/vidhrdw.c
        mvs/${PLATFORM_LOWER}_sprite.c
        mvs/pd4990a.c
        mvs/neocrypt.c
        mvs/biosmenu.c
        sound/2610intf.o 
        sound/ym2610.c
    )
endif()

set(ALL_SRC ${COMMON_SRC} ${OS_SRC} ${TARGET_SRC})

list(TRANSFORM ALL_SRC PREPEND "src/")
# # Create the executable
add_executable(${TARGET}
    ${ALL_SRC}
)

# Add the warning flags
target_compile_options(${TARGET} PUBLIC ${WARNING_OPTIONS})

# Add common libraries
target_link_libraries(${TARGET} PUBLIC
    z
)