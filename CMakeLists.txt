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
set(VITA_VERSION  "01.00")
set(VITA_MKSFOEX_FLAGS "-d ATTRIBUTE2=12")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,-q,--wrap,memcpy,--wrap,memmove,--wrap,memset -D_GNU_SOURCE -Wall -O3 -mfloat-abi=softfp")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11")

add_executable(HOMM3.elf
  loader/main.c
  loader/dialog.c
  loader/so_util.c
)

target_link_libraries(HOMM3.elf
  -Wl,--whole-archive pthread -Wl,--no-whole-archive
  stdc++
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
  theora
  jpeg
  png
  webp
  m
  z
  kubridge_stub
  SceAppMgr_stub
  SceAudio_stub
  SceAudioIn_stub
  SceCtrl_stub
  SceCommonDialog_stub
  SceDisplay_stub
  SceIme_stub
  SceKernelDmacMgr_stub
  SceGxm_stub
  SceHid_stub
  SceSysmodule_stub
  ScePower_stub
  SceTouch_stub
  SceMotion_stub
  SceVshBridge_stub
  SceShaccCg_stub
)

vita_create_self(eboot.bin HOMM3.elf UNSAFE)
vita_create_vpk(HOMM3.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE ${CMAKE_SOURCE_DIR}/sce_sys/icon0.png sce_sys/icon0.png
       ${CMAKE_SOURCE_DIR}/sce_sys/pic0.png sce_sys/pic0.png
       ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
       ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/bg0.png sce_sys/livearea/contents/bg0.png
       ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
       ${CMAKE_SOURCE_DIR}/module/libGLESv2.suprx module/libGLESv2.suprx
       ${CMAKE_SOURCE_DIR}/module/libgpu_es4_ext.suprx module/libgpu_es4_ext.suprx
       ${CMAKE_SOURCE_DIR}/module/libIMGEGL.suprx module/libIMGEGL.suprx
       ${CMAKE_SOURCE_DIR}/module/libpvrPSP2_WSEGL.suprx module/libpvrPSP2_WSEGL.suprx
)
