package com.khopan.hackontrol.channel;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.button.ButtonHandlerRegistry;

public class FileChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "file";

	@Override
	public String getChannelName() {
		return FileChannel.CHANNEL_NAME;
	}

	@Override
	public void sendInitializeMessage() {

	}

	@Override
	public void registerButtonHandler(ButtonHandlerRegistry registry) {

	}
}
