#include "iio_client.h"
#include <iostream>
#include <string.h>
#include <errno.h>

IIOClient::IIOClient(const char* deviceName, const char* triggerName) {
	ctx_ = iio_create_default_context();
	if (!ctx_) {
		throw std::runtime_error("Unable to create IIO context: " + std::string(strerror(errno)));
	}

	dev_ = iio_context_find_device(ctx_, deviceName);
	if (!dev_) {
		throw std::runtime_error("Device not found: " + std::string(strerror(errno)));
	}

	trigger_ = iio_context_find_device(ctx_, triggerName);
	if (!trigger_ || !iio_device_is_trigger(trigger_)) {
		throw std::runtime_error("No trigger found (try setting up the iio-trig-hrtimer module): " + std::string(strerror(errno)));
	}

	if (iio_device_set_trigger(dev_, trigger_)) {
		throw std::runtime_error("Could not set trigger: " + std::string(strerror(errno)));
	}
}

bool IIOClient::ReadParameter(const char* channelName, const char* attrName, char* dst, size_t len) {
	if (ctx_ && dev_) {
		channel_ = iio_device_find_channel(dev_, channelName, false);
		if (!channel_) {
			throw std::runtime_error("Unable to find input channel: " + std::string(strerror(errno)));
		}

		iio_channel_enable(channel_);

		ssize_t bytes_read = iio_channel_attr_read(channel_, attrName, dst, len);
		
		iio_channel_disable(channel_);

		if (bytes_read > 0) {
			return true;
		}
	}
	
	return false;
}

bool IIOClient::WriteParameter(const char* channelName, const char* attrName, const char* src) {
	if (ctx_ && dev_) {
		channel_ = iio_device_find_channel(dev_, channelName, true);
		if (!channel_) {
			throw std::runtime_error("Unable to find output channel: " + std::string(strerror(errno)));
		}

		iio_channel_enable(channel_);

		ssize_t bytes_written = iio_channel_attr_write(channel_, attrName, src);

		iio_channel_disable(channel_);

		if (bytes_written > 0) {
			return true;
		}
	}

	return false;
}

IIOClient::~IIOClient() {
	if (dev_) {
		int ret = iio_device_set_trigger(dev_, NULL);
		if (ret < 0) {
			char buf[256];
			iio_strerror(-ret, buf, sizeof(buf));
			std::cerr << "Trigger disassociating failed: " << buf << std::endl;
		}
	}
	if (ctx_)
		iio_context_destroy(ctx_);
}