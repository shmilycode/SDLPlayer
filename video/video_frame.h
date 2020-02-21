#pragma once
#ifndef STREAM_HUB_VIDEO_VIDEO_FRAME_H_
#define STREAM_HUB_VIDEO_VIDEO_FRAME_H_

#include <stdint.h>
#include <memory>
#include <cstdio>

namespace stream_hub {

	struct VideoFrameBaseInfo {
		int width;

		int height;

		int pixel_format;
	};

	class VideoFrame {
	public:
		virtual ~VideoFrame() {
			if (buffer)
				delete(buffer);
		}
		enum {
			kMaxPlanes = 8,
			kYPlane = 0,
			kARGBPlane = kYPlane,
			kUPlane = 1,
			kUVPlane = kUPlane,
			kVPlane = 2,
			kAPlane = 3,
		};

		uint8_t* data[kMaxPlanes];

		int linesize[kMaxPlanes];

		uint8_t **extended_data;

		VideoFrameBaseInfo base_info;

		int key_frame;

		int64_t pts;

		int sample_rate;

		size_t crop_top;

		size_t crop_bottom;

		size_t crop_left;

		size_t crop_right;

		uint8_t* buffer = nullptr;
	};

} //namespace stream_hub

#endif
