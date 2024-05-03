package com.khopan.hackontrol.channel;

import com.khopan.hackontrol.HackontrolChannel;

public class HackontrolDiscordChannel extends HackontrolChannel {
	private static String CHANNEL_NAME = "hackontrol";

	@Override
	public String getName() {
		return HackontrolDiscordChannel.CHANNEL_NAME;
	}
}
