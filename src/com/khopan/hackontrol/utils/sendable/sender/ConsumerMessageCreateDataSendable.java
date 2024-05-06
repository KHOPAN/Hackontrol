package com.khopan.hackontrol.utils.sendable.sender;

import java.util.function.Consumer;

import com.khopan.hackontrol.utils.sendable.ISendable;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class ConsumerMessageCreateDataSendable implements ISendable {
	private final Consumer<MessageCreateData> consumer;
	private final Consumer<Consumer<Message>> callback;

	private ConsumerMessageCreateDataSendable(Consumer<MessageCreateData> consumer, Consumer<Consumer<Message>> callback) {
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
		return new ConsumerMessageCreateDataSendable(consumer, callback);
	}

	public static ISendable of(Consumer<MessageCreateData> consumer) {
		return ConsumerMessageCreateDataSendable.of(consumer, null);
	}
}
