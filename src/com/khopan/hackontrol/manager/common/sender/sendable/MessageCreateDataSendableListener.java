package com.khopan.hackontrol.manager.common.sender.sendable;

import java.util.function.Consumer;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class MessageCreateDataSendableListener implements ISendable {
	private final Consumer<MessageCreateData> consumer;
	private final Consumer<Consumer<Message>> callback;

	private MessageCreateDataSendableListener(Consumer<MessageCreateData> consumer, Consumer<Consumer<Message>> callback) {
		this.consumer = consumer;
		this.callback = callback;
	}

	@Override
	public void send(MessageCreateData message, Consumer<Message> callback) {
		this.consumer.accept(message);

		if(this.callback != null) {
			this.callback.accept(callback);
		}
	}

	public static ISendable of(Consumer<MessageCreateData> consumer, Consumer<Consumer<Message>> callback) {
		return new MessageCreateDataSendableListener(consumer, callback);
	}

	public static ISendable of(Consumer<MessageCreateData> consumer) {
		return MessageCreateDataSendableListener.of(consumer, null);
	}
}
