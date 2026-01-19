cmake_minimum_required(VERSION 3.22)

project(DevHelper LANGUAGES CXX C VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_program(MAKEROM makerom)
find_program(BANNERTOOL bannertool)

# Set Result specific Data
set(APP_NAME "DevHelper")
set(APP_DESC "NPI-D7 fast dev apps updeter.")
set(APP_AUTHOR "npid7")
set(APP_ICON "${CMAKE_SOURCE_DIR}/app/icon.png")
set(APP_ROMFS "${CMAKE_SOURCE_DIR}/romfs")

set(APP_BANNER "${CMAKE_SOURCE_DIR}/app/banner.png")
set(APP_BANNERAUDIO "${CMAKE_SOURCE_DIR}/app/BannerAudio.wav")
set(APP_RSF "${CMAKE_SOURCE_DIR}/app/build-cia.rsf")
set(APP_LOGO "${CMAKE_SOURCE_DIR}/app/splash.lz")

add_executable(${PROJECT_NAME}
    source/cia.cpp
    source/download.cpp
    source/Files.cpp
    source/main.cpp
    source/nightlyreader.cpp
    source/Ovls.cpp
    source/scenes.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    libs/include
    $ENV{DEVKITPRO}/portlibs/3ds/include
)
target_link_directories(${PROJECT_NAME} PRIVATE libs/lib)
target_link_libraries(${PROJECT_NAME} PRIVATE
    renderd7
    curl
    mbedtls
    mbedx509
    mbedcrypto
    bz2
    lzma
    z
    citro2d
    citro3d
    ctru
    m
)

# Generate 3DSX
ctr_generate_smdh(
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.smdh
    NAME "${APP_NAME}"
    DESCRIPTION "${APP_DESC}"
    AUTHOR "${APP_AUTHOR}"
    ICON "${APP_ICON}"
)
ctr_create_3dsx(
    ${PROJECT_NAME}
    OUTPUT "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.3dsx"
    SMDH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.smdh"
    ROMFS "${APP_ROMFS}"
)

if(MAKEROM AND BANNERTOOL)
    message(STATUS "Found Makerom and Bannertool. Building Cia...")

    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bnr
        COMMAND ${BANNERTOOL} makebanner
        -i ${APP_BANNER}
        -a ${APP_BANNERAUDIO}
        -o ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bnr
        DEPENDS ${APP_BANNER} ${APP_BANNERAUDIO}
        COMMENT "Creating Banner for cia file..."
        VERBATIM
    )

    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.cia
        COMMAND ${MAKEROM}
        -f cia -target t -exefslogo
        -o "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.cia"
        -elf "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf"
        -rsf ${APP_RSF}
        -banner "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bnr"
        -icon "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.smdh"
        -logo ${APP_LOGO} -DAPP_ROMFS="${APP_ROMFS}"
        -major 2 -minor 0 -micro 0
        -DAPP_VERSION_MAJOR=1
        DEPENDS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.smdh
        COMMENT "Creating Cia file..."
        VERBATIM
    )

    add_custom_target(make_banner ALL
        DEPENDS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bnr
    )

    add_custom_target(make_cia ALL
        DEPENDS make_banner ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.cia
    )
endif()


find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    message(STATUS "Clang Format found.")
    set(SOURCES ${CMAKE_SOURCE_DIR}/source ${CMAKE_SOURCE_DIR}/include)
    set(COMMENT "Formatting Project Sources")
    if(WIN32)
        add_custom_target(clang-format
            COMMAND powershell.exe -Command "Get-ChildItem '${SOURCES}/*' -Include *.cpp,*.hpp -Recurse | Foreach {&'${CLANG_FORMAT}' -i $_.fullname}"
            COMMENT ${CCOMMENT})
    else()
        add_custom_target(clang-format
            COMMAND find ${SOURCES} -iname *.hpp -o -iname *.cpp | xargs ${CLANG_FORMAT} -i
            COMMENT ${COMMENT}
        )
    endif()
    unset(COMMENT)
endif()
