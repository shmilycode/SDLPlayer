#pragma once

#include "video_factory.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include "video_frame.h"
#include "video_stream.h"

using stream_hub::VideoFrame;
using stream_hub::VideoStream;

namespace VideoPlayer {
class MulticastVideoFactory: public VideoFactory {
 public:
  MulticastVideoFactory();

  ~MulticastVideoFactory() override;

  void OpenStream(const std::string& multicast_address); 

  void CloseStream();

  std::unique_ptr<VideoFrame> GetAvailableFrame() override;

 protected:
  void OnError(int code, const char* message);

  void OnFrameReady(std::unique_ptr<VideoFrame> frame);

  void OnFrameRateChanged(int total, int dropped, int decoded);

 private:
  std::unique_ptr<VideoStream> video_stream_;

  std::mutex mutex_;

  std::condition_variable cond_;

  std::queue<std::unique_ptr<VideoFrame>> frame_queue_;
};
}