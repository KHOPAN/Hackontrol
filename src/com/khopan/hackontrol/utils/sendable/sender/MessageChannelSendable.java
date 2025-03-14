package com.khopan.hackontrol.utils.sendable.sender;

import java.util.function.Consumer;

import com.khopan.hackontrol.utils.sendable.ISendable;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class MessageChannelSendable implements ISendable {
	private final MessageChannel channel;

	private MessageChannelSendable(MessageChannel channel) {
		this.channel = channel;
	}

	@Override
	public void send(MessageCreateData message, Consumer<Message> callback) {
		this.channel.sendMessage(message).queue(callback);
	}

	public static ISendable of(MessageChannel channel) {
		return new MessageChannelSendable(channel);
	}
}
