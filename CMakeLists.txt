cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(HOMM3.elf C CXX)
include("${VITASDK}/share/vita.cmake" REQUIRED)
set(VITA_APP_NAME "Heroes of Might and Magic 3 HD")
set(VITA_TITLEID  "HOMM00003")
set(VITA_VERSION  "01.20")
set(VITA_MKSFOEX_FLAGS "-d ATTRIBUTE2=12")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g -O3 -Wno-strict-aliasing \
                  -Wl,-q,--wrap,memcpy,--wrap,memmove,--wrap,memset,--wrap,free,--wrap,malloc,--wrap,calloc,--wrap,realloc,--wrap,memalign")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11")

add_executable(HOMM3.elf
  loader/main.c
  loader/input.c
  loader/so_util.c
  loader/reimpl/memory.c
  loader/reimpl/pthr.c
  loader/reimpl/other.c
  loader/utils/dialog.c
  loader/utils/gxm_dialog.c
  loader/utils/utils.c
  loader/utils/trophies.c
)

target_include_directories(HOMM3.elf
  PUBLIC ${CMAKE_SOURCE_DIR}/loader
)

if(GRAPHICS_API_PVR)
  add_definitions(-DGRAPHICS_API_PVR=1)
  target_link_libraries(HOMM3.elf
    pthread
    SDL2_mixer
    SDL2_image
    SDL2
    vorbis
    ogg
    vorbisfile
    FLAC
    mikmod
    mpg123
    modplug
    theora
    jpeg
    png
    webp
    libGLESv2_stub
    libgpu_es4_ext_stub
    libIMGEGL_stub
    m
    z
    stdc++
    taihen_stub
    kubridge_stub
    SceAppMgr_stub
    SceAppUtil_stub
    SceAudio_stub
    SceAudioIn_stub
    SceCtrl_stub
    SceCommonDialog_stub
    SceDisplay_stub
    SceIme_stub
    SceGxm_stub
    SceHid_stub
    ScePower_stub
    SceTouch_stub
    SceMotion_stub
    SceVshBridge_stub
    SceNpTrophy_stub
  )
else()
  add_definitions(-DGRAPHICS_API_VITAGL=1)
  target_link_libraries(HOMM3.elf
    pthread
    SDL2_mixer
    SDL2_image
    SDL2
    vitaGL
    vitashark
    mathneon
    vorbis
    ogg
    vorbisfile
    FLAC
    mikmod
    mpg123
    modplug
    theora
    jpeg
    png
    webp
    m
    z
    stdc++
    SceShaccCgExt
    taihen_stub
    kubridge_stub
    SceAppMgr_stub
    SceAppUtil_stub
    SceAudio_stub
    SceAudioIn_stub
    SceCtrl_stub
    SceCommonDialog_stub
    SceDisplay_stub
    SceIme_stub
    SceGxm_stub
    SceHid_stub
    ScePower_stub
    SceTouch_stub
    SceMotion_stub
    SceVshBridge_stub
    SceNpTrophy_stub
    SceShaccCg_stub
    SceKernelDmacMgr_stub
  )
endif()

add_executable(config
  config/main.cpp
)

target_link_libraries(config
  imgui
  vitaGL
  vitashark
  SceShaccCgExt
  m
  mathneon
  taihen_stub
  SceAppMgr_stub
  SceAudio_stub
  SceCtrl_stub
  SceCommonDialog_stub
  SceDisplay_stub
  SceKernelDmacMgr_stub
  SceGxm_stub
  SceShaccCg_stub
  SceTouch_stub
  SceVshBridge_stub
)

vita_create_self(config.bin config UNSAFE)

vita_create_self(eboot.bin HOMM3.elf UNSAFE)

if(GRAPHICS_API_PVR)
  vita_create_vpk(HOMM3.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE ${CMAKE_SOURCE_DIR}/sce_sys/icon0.png sce_sys/icon0.png
        ${CMAKE_SOURCE_DIR}/sce_sys/pic0.png sce_sys/pic0.png
        ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
        ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/bg0.png sce_sys/livearea/contents/bg0.png
        ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
        ${CMAKE_SOURCE_DIR}/sce_sys/trophy/HOMM00003_00/TROPHY.TRP sce_sys/trophy/HOMM00003_00/TROPHY.TRP
        ${CMAKE_SOURCE_DIR}/module/libGLESv2.suprx module/libGLESv2.suprx
        ${CMAKE_SOURCE_DIR}/module/libgpu_es4_ext.suprx module/libgpu_es4_ext.suprx
        ${CMAKE_SOURCE_DIR}/module/libIMGEGL.suprx module/libIMGEGL.suprx
        ${CMAKE_SOURCE_DIR}/module/libpvrPSP2_WSEGL.suprx module/libpvrPSP2_WSEGL.suprx
        ${CMAKE_SOURCE_DIR}/module/libgpu_es4_kernel_ext.skprx module/libgpu_es4_kernel_ext.skprx
        ${CMAKE_SOURCE_DIR}/data/cursor.png data/cursor.png
        ${CMAKE_BINARY_DIR}/config.bin config.bin
  )
else()
  vita_create_vpk(HOMM3.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE ${CMAKE_SOURCE_DIR}/sce_sys/icon0.png sce_sys/icon0.png
        ${CMAKE_SOURCE_DIR}/sce_sys/pic0.png sce_sys/pic0.png
        ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
        ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/bg0.png sce_sys/livearea/contents/bg0.png
        ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
        ${CMAKE_SOURCE_DIR}/sce_sys/trophy/HOMM00003_00/TROPHY.TRP sce_sys/trophy/HOMM00003_00/TROPHY.TRP
        ${CMAKE_SOURCE_DIR}/data/cursor.png data/cursor.png
        ${CMAKE_BINARY_DIR}/config.bin config.bin
  )
endif()
