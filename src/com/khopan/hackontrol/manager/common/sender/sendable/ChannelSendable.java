package com.khopan.hackontrol.manager.common.sender.sendable;

import java.util.function.Consumer;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class ChannelSendable implements ISendable {
	private final MessageChannel channel;

	private ChannelSendable(MessageChannel channel) {
		this.channel = channel;
	}

	@Override
	public void send(MessageCreateData message, Consumer<Message> callback) {
		this.channel.sendMessage(message).queue(callback);
	}

	public static ISendable of(MessageChannel channel) {
		return new ChannelSendable(channel);
	}
}
