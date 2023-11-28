package com.khopan.hackontrol.source;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;

public class DefaultCommandSource implements CommandSource {
	private final MessageChannel channel;

	public DefaultCommandSource(MessageChannel channel) {
		this.channel = channel;
	}

	@Override
	public void sendMessage(String message) {
		this.channel.sendMessage(message).queue();
	}
}
