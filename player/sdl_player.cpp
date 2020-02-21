#include "sdl_player.h"

#include <algorithm>
#include <assert.h>

#include "logger.h"
#include "video/video_factory.h"

namespace {
/* polls for possible required screen refresh at least this often, should be less than 1/fps */
constexpr float kRefreshRate = 0.1; //
}

/* Fast a/(1<<b) rounded toward +inf. Assume a>=0 and b>=0 */
#define AV_CEIL_RSHIFT(a,b) (!__builtin_constant_p(b) ? -((-(a)) >> (b)) \
                                                       : ((a) + (1<<(b)) - 1) >> (b))

namespace VideoPlayer {
SDLPlayer::SDLPlayer(SDLConfig cfg, VideoFactory* video_factory)
    :video_factory_(video_factory),
    is_initialized_(false),
    window_(nullptr),
    renderer_(nullptr),
    texture_(nullptr),
    window_width_(cfg.window_width),
    window_height_(cfg.window_height),
    pixel_format_(cfg.pixel_format),
    stoped_(true) {}

SDLPlayer::~SDLPlayer() {
  DestroyWindow();
  SDL_Quit();
}

bool SDLPlayer::Initialize() {
  assert(video_factory_);
  // return zero on success else non-zero 
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { 
      LOG_ERROR << "Could not initialize SDL - " << SDL_GetError();
      return false;
  }

  //SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
  //SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
  return true;
}

bool SDLPlayer::SetWindow(uint16_t window_width, uint16_t window_height, const std::string& window_title) {
  if (!window_) {
    int flags = SDL_WINDOW_SHOWN;
    //flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    //flags |= SDL_WINDOW_BORDERLESS;
    flags |= SDL_WINDOW_RESIZABLE;
    // create a window
    window_ = SDL_CreateWindow(window_title.c_str(), 
                              SDL_WINDOWPOS_UNDEFINED, 
                              SDL_WINDOWPOS_UNDEFINED, 
                              window_width, 
                              window_height, 
                              flags);
    LOG_INFO << "Initialized windows.";
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    if (window_) {
        // creates a renderer to render our frame.
        // Try hardware accelerated first.
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer_) {
            LOG_WARN << "Failed to initialize a hardware accelerated renderer: " <<  SDL_GetError();
            // default render.
            renderer_ = SDL_CreateRenderer(window_, -1, 0);
        }
        if (renderer_) {
            SDL_RendererInfo info;
            if (!SDL_GetRendererInfo(renderer_, &info))
                LOG_INFO << "Initialized " << info.name << " renderer.";
        }
    }
  } else { // window has been created, just resize it.
      SDL_SetWindowSize(window_, window_width, window_height);
  }

  if (!window_ || !renderer_) {
    LOG_FATAL << "SDL: could not set video mode.";
    DestroyWindow();

    return false;
  }
  return true;
}

void SDLPlayer::DestroyWindow() {
  if (renderer_) {
      SDL_DestroyRenderer(renderer_);
      renderer_ = nullptr;
  }
  if (window_) {
      SDL_DestroyWindow(window_);
      window_ = nullptr;
  }
}

bool SDLPlayer::Play() {
  if (!is_initialized_) {
    if (Initialize()) {
      is_initialized_ = true;
    } else {
      return false;
    }
  }
  stoped_ = false;
  render_thread_ = std::move(std::thread(&SDLPlayer::EventLoop, this));
  return true;
}

void SDLPlayer::Stop() {
  stoped_ = true;
  if (render_thread_.joinable())
    render_thread_.join();
}

void SDLPlayer::EventLoop() {
  SDL_Event event;
  while(!stoped_) {
    RefreshLoopWaitEvent(&event);
    switch (event.type) {
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      default:
        break;
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      break;
    case SDL_MOUSEMOTION:
      break;
    case SDL_WINDOWEVENT:
      switch (event.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
          // We've handle in @ReallocTexture
          break;
        case SDL_WINDOWEVENT_EXPOSED:
          break;
      }
      break;
    case SDL_QUIT:
    default:
        break;
    }
  }
}

void SDLPlayer::RefreshLoopWaitEvent(SDL_Event *event) {
    double remaining_time = 0.0;

    // Try get event or display frame.
    SDL_PumpEvents();
    while (!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
        if (remaining_time > 0.0) {
          std::this_thread::sleep_for(
              std::chrono::milliseconds(10));
        }
        remaining_time = kRefreshRate;
        // Core
        RefreshVideo(remaining_time);
        SDL_PumpEvents();
    }
}

void SDLPlayer::RefreshVideo(double& remaining_time) {
   /* display picture */
   if (!stoped_)
       DisplayFrame();
}

void SDLPlayer::DisplayFrame() {
    if (!window_) {
      if (!SetWindow(window_width_, window_height_, "SDL Player")) {
        return;
      }
    }

    std::unique_ptr<stream_hub::VideoFrame> frame = video_factory_->GetAvailableFrame();
    if (frame) {
      SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
      SDL_RenderClear(renderer_);
      VideoImageDisplay(std::move(frame));
      SDL_RenderPresent(renderer_);
    }
}

void SDLPlayer::VideoImageDisplay(std::unique_ptr<stream_hub::VideoFrame> frame) {
  SDL_Rect rect;
  CalculateDisplayRect(&rect, 0, 0, window_width_, window_height_, frame->base_info.width, frame->base_info.height, 1.0);
  if (UploadTexture(frame.get()) < 0)
     return;
  // Triggers the double buffers for multiple rendering 
  SDL_RenderCopyEx(renderer_, texture_, NULL, &rect, 0, NULL, SDL_FLIP_NONE);
}

void SDLPlayer::CalculateDisplayRect(SDL_Rect *rect,
                                   int scr_xleft, int scr_ytop, int scr_width, int scr_height,
                                   int pic_width, int pic_height, float aspect_ratio) {
    int width, height, x, y;

    if (aspect_ratio <= 0.0)
        aspect_ratio = 1.0;
    aspect_ratio *= (float)pic_width / (float)pic_height;

    /* We suppose the screen has a 1.0 pixel ratio */
    height = scr_height;
    width = lrint(height * aspect_ratio) & ~1;
    if (width > scr_width) {
        width = scr_width;
        height = lrint(width / aspect_ratio) & ~1;
    }
    x = (scr_width - width) / 2;
    y = (scr_height - height) / 2;
    rect->x = scr_xleft + x;
    rect->y = scr_ytop  + y;
    rect->w = std::max(width,  1);
    rect->h = std::max(height, 1);
}

int SDLPlayer::UploadTexture(stream_hub::VideoFrame *frame) {
    int ret = 0;
    // Only support pixel format SDL_PIXELFORMAT_IYUV.
    uint32_t sdl_pix_fmt = pixel_format_;
    SDL_BlendMode sdl_blendmode = SDL_BLENDMODE_NONE;
    if (ReallocTexture(sdl_pix_fmt, frame->base_info.width, frame->base_info.height, sdl_blendmode) < 0)
        return -1;
    if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0) {
        ret = SDL_UpdateYUVTexture(texture_, NULL, frame->data[0], frame->linesize[0],
                                               frame->data[1], frame->linesize[1],
                                               frame->data[2], frame->linesize[2]);
    } else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0) {
        ret = SDL_UpdateYUVTexture(texture_, NULL, frame->data[0] + frame->linesize[0] * (frame->base_info.height                    - 1), -frame->linesize[0],
                                               frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->base_info.height, 1) - 1), -frame->linesize[1],
                                               frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->base_info.height, 1) - 1), -frame->linesize[2]);
    } else {
        LOG_ERROR << "Mixed negative and positive linesizes are not supported.";
        return -1;
    }
    return ret;
}

int SDLPlayer::ReallocTexture(uint32_t new_format, int new_width, int new_height, SDL_BlendMode blendmode)
{
    uint32_t format;
    int access, w, h;
    if (SDL_QueryTexture(texture_, &format, &access, &w, &h) < 0 || new_width != w || new_height != h || new_format != format) {
        void *pixels;
        int pitch;
        SDL_DestroyTexture(texture_);
        if (!(texture_ = SDL_CreateTexture(renderer_, new_format, SDL_TEXTUREACCESS_STREAMING, new_width, new_height))) {
            LOG_ERROR << "Create texture failed: " << SDL_GetError();
            return -1;
        }
        if (SDL_SetTextureBlendMode(texture_, blendmode) < 0)
            return -1;
        LOG_INFO << "Initialized " << new_width << ":" << new_height << " texture with " << SDL_GetPixelFormatName(new_format);
    }
    return 0;
}

} //namespace
