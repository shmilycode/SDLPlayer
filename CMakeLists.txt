cmake_minimum_required (VERSION 3.4)

project ("sdl_player")

include_directories(${PROJECT_SOURCE_DIR})
link_directories(${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_BUILD_TYPE "Debug")
set (BASE_SOURCE_FILE
  player/sdl_player.cpp
  player/sdl_player.h
  video/multicast_video_factory.cpp
  video/multicast_video_factory.h
  video/video_factory.h
  logger.h
  main.cpp
)

add_executable(player ${BASE_SOURCE_FILE})
target_link_libraries(player multicast SDL2 pthread)

# for static build.
#target_link_libraries(player multicast avformat avcodec avutil swscale -ldl -static-libgcc -static-libstdc++ -Wall -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive)
