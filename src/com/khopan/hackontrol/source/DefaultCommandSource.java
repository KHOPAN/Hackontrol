package com.khopan.hackontrol.source;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;

public class DefaultCommandSource implements CommandSource {
	private final String machineIdentifier;
	private final MessageChannel channel;

	public DefaultCommandSource(String machineIdentifier, MessageChannel channel) {
		this.machineIdentifier = machineIdentifier;
		this.channel = channel;
	}

	@Override
	public String getMachineId() {
		return this.machineIdentifier;
	}

	@Override
	public void sendMessage(String message) {
		this.channel.sendMessage(message).queue();
	}

	@Override
	public void sendBoldCodeMessage(String message) {
		this.sendMessage("**`" + message + "`**");
	}
}
