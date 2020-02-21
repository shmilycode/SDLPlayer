#pragma once

#include <memory>
#include "video/video_frame.h"

namespace VideoPlayer {
class VideoFactory {
 public:
  virtual ~VideoFactory() {}

  // Get one available frame to render from factory.
  // Function may block to wait frame or return null.
  virtual std::unique_ptr<stream_hub::VideoFrame> GetAvailableFrame() = 0;
};
}