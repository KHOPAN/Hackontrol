package com.khopan.hackontrol.channel;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.registry.Registry;

public class FileChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "file";

	@Override
	public String getName() {
		return FileChannel.CHANNEL_NAME;
	}

	@Override
	public void register(Registry registry) {

	}
}
