#pragma once
#include <iio.h>

class IIOClient
{
public:
	IIOClient(const char* deviceName, const char* triggerName);
	bool ReadParameter(const char* channelName, const char* attrName, char* dst, size_t len);
	bool WriteParameter(const char* channelName, const char* attrName, const char* src);
	~IIOClient();

private:
	struct iio_context* ctx_;
	struct iio_device* dev_, *trigger_;
	struct iio_channel* channel_;
};