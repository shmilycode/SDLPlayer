#pragma once
#include <SDL2/SDL.h>

#include <memory>
#include <string>
#include <thread>
#include "video/video_frame.h"

namespace VideoPlayer {

struct SDLConfig {
  uint16_t window_width;
  uint16_t window_height;
  uint32_t pixel_format;
};
class VideoFactory;
class SDLPlayer {
 public:
  SDLPlayer(SDLConfig cfg, VideoFactory* video_factory);

  ~SDLPlayer();

  // Start play.
  bool Play();

  // Stop play.
  void Stop();

 private:
  // Initialize SDL.
  bool Initialize();

  // Destroy window and render. 
  void DestroyWindow();

  // Create window with specified size and title, if window was created, just change its size.
  bool SetWindow(uint16_t window_width, uint16_t window_height, const std::string& window_title);

  // Handle SDL event in loop.
  void EventLoop();

  // Get SDL event in loop.
  void RefreshLoopWaitEvent(SDL_Event *event);

  // Called to display each frame
  void RefreshVideo(double& remaining_time);

  // Display the current picture, if any
  void DisplayFrame();

  // Fill texture and do render.
  void VideoImageDisplay(std::unique_ptr<stream_hub::VideoFrame> frame);

  void CalculateDisplayRect(SDL_Rect *rect,
                            int scr_xleft, int scr_ytop, int scr_width, int scr_height,
                            int pic_width, int pic_height, float aspect_ratio);

  int UploadTexture(stream_hub::VideoFrame *frame);

  int ReallocTexture(uint32_t new_format, int new_width, int new_height, SDL_BlendMode blendmode);

  inline double rint(double x) {return x >= 0 ? floor(x + 0.5) : ceil(x - 0.5);}

 private:
  VideoFactory* video_factory_;

  bool is_initialized_;

  SDL_Window *window_;

  SDL_Renderer *renderer_;

  SDL_Texture *texture_;

  uint16_t window_width_;

  uint16_t window_height_;

  uint32_t pixel_format_;

  volatile bool stoped_;

  std::thread render_thread_;
};

}//namespace