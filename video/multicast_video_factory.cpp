#include "multicast_video_factory.h"

#include <chrono>
#include "logger.h"

namespace VideoPlayer {
MulticastVideoFactory::MulticastVideoFactory()
  :video_stream_(nullptr) {}

MulticastVideoFactory::~MulticastVideoFactory() {
  if (video_stream_) {
    video_stream_->CloseStream();
    video_stream_ = nullptr;
  }

}

void MulticastVideoFactory::OpenStream(const std::string& multicast_address) {
  if (video_stream_) {
    return;
  }
  video_stream_ = std::make_unique<VideoStream>();
  video_stream_->SetErrorCallback(
      std::bind(&MulticastVideoFactory::OnError, 
        this, 
        std::placeholders::_1, 
        std::placeholders::_2));
  video_stream_->SetFrameCallback(
      std::bind(&MulticastVideoFactory::OnFrameReady,
        this,
        std::placeholders::_1));
  stream_hub::SetLogCallback(
  [](const char *file, int line, const char *func, int severity, const char *content){
    if(severity > 0)
      LOG_DEBUG << content;
  });

  video_stream_->SetFrameRateCallback(
      std::bind(&MulticastVideoFactory::OnFrameRateChanged,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3));
  video_stream_->OpenStream(multicast_address.c_str(), "");
}

void MulticastVideoFactory::CloseStream() {
  if (video_stream_) {
    video_stream_->CloseStream();
    video_stream_ = nullptr;
    LOG_DEBUG << "After close stream";
  }
}

std::unique_ptr<VideoFrame> MulticastVideoFactory::GetAvailableFrame() {
  std::unique_lock<std::mutex> lock_(mutex_);
  while (frame_queue_.empty()) {
    if(cond_.wait_for(lock_, std::chrono::milliseconds(200)) == std::cv_status::timeout){
      return nullptr;
    }
  }
  std::unique_ptr<VideoFrame> frame = std::move(frame_queue_.front());
  frame_queue_.pop();
  return std::move(frame);
}

void MulticastVideoFactory::OnError(int code, const char* message) {
  LOG_ERROR << message;
}

void MulticastVideoFactory::OnFrameReady(std::unique_ptr<VideoFrame> frame) {
  std::lock_guard<std::mutex> lock_(mutex_);
  frame_queue_.push(std::move(frame));
}

void MulticastVideoFactory::OnFrameRateChanged(int total, int dropped, int decoded) {
  LOG_DEBUG << decoded;
}
}
