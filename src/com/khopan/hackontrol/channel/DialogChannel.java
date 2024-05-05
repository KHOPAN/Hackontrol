package com.khopan.hackontrol.channel;

import com.khopan.hackontrol.HackontrolChannel;

public class DialogChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "dialog";

	@Override
	public String getName() {
		return DialogChannel.CHANNEL_NAME;
	}
}
