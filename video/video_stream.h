#pragma once

#include <functional>
#include <mutex>
#include <memory>

#define EXPORT __attribute__((visibility("default")))
using MLogCallBack = std::function<void(const char *file, int line, const char *func, int severity, const char *content)>;
class VideoReceiver;
namespace stream_hub {

class VideoClient;
class VideoFrame;
class VideoInputDevice;
class VideoReceiveImageObserver;

EXPORT void SetLogCallback(MLogCallBack logcallback);

EXPORT void SetExceptionDumpPath(const char* path);

class EXPORT VideoStream {
public:
  using OnErrorCallback = std::function<void(int, const char*)>;
  using OnFrameReadyCallback = std::function<void(std::unique_ptr<VideoFrame>)>;
  using OnFrameRateCallback = std::function<void(int, int, int)>;

  VideoStream();

  ~VideoStream(); 

  void OpenStream(const char* source, const char* local);

  void SetErrorCallback(OnErrorCallback error_callback);

  void SetFrameCallback(OnFrameReadyCallback frame_callback);

  void CloseStream();

  void SetFrameRateCallback(OnFrameRateCallback frame_count_callback);

private:
  std::mutex stream_lock_;
  std::unique_ptr<VideoInputDevice> video_input_device_;
  std::unique_ptr<VideoReceiveImageObserver> decoder_image_provider_;;
  std::unique_ptr<VideoReceiver> video_receiver_;
  std::unique_ptr<VideoClient> video_client_;
};


} // namespace stream_hub
