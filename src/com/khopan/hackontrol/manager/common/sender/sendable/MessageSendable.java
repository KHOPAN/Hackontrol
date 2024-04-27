package com.khopan.hackontrol.manager.common.sender.sendable;

import java.util.function.Consumer;

import com.khopan.hackontrol.manager.common.sender.IMessageable;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class MessageSendable implements ISendable {
	private final IMessageable messageable;

	private MessageSendable(IMessageable messageable) {
		this.messageable = messageable;
	}

	@Override
	public void send(MessageCreateData message, Consumer<Message> callback) {
		this.messageable.sendMessage(message).queue(callback);
	}

	public static ISendable of(IMessageable messageable) {
		return new MessageSendable(messageable);
	}
}
