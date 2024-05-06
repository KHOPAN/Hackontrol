package com.khopan.hackontrol.utils;

import java.util.function.Consumer;

import com.khopan.hackontrol.manager.interaction.InteractionManager;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.interactions.InteractionHook;
import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class TimeSafeReplyHandler {
	private static int Count;

	private final IReplyCallback callback;
	private final Consumer<Message> consumer;

	private volatile MessageCreateData message;
	private volatile InteractionHook hook;
	private volatile boolean processed;
	private volatile boolean defer;

	private TimeSafeReplyHandler(IReplyCallback callback, long timeout, Consumer<Consumer<MessageCreateData>> action, Consumer<Message> consumer) {
		long start = System.currentTimeMillis();
		this.callback = callback;
		this.consumer = consumer;
		Thread thread = new Thread(() -> {
			action.accept(this :: callback);
			TimeSafeReplyHandler.Count--;
		});

		thread.setName("Hackontrol Time-Safe Reply Handler #" + (++TimeSafeReplyHandler.Count));
		thread.start();

		try {
			Thread.sleep(timeout - (System.currentTimeMillis() - start));
		} catch(Throwable Errors) {
			Errors.printStackTrace();
		}

		if(this.message == null) {
			this.defer = true;
			callback.deferReply().queue(this :: hook);
		}
	}

	private void callback(MessageCreateData message) {
		this.message = message;

		if(this.defer) {
			if(this.hook != null) {
				this.execute();
			}
		} else {
			this.execute();
		}
	}

	private void hook(InteractionHook hook) {
		this.hook = hook;

		if(this.message != null) {
			this.execute();
		}
	}

	private void execute() {
		if(this.processed) {
			return;
		}

		if(!this.defer) {
			this.callback.reply(this.message).queue(this :: consumeReply);
			this.processed = true;
			return;
		}

		this.hook.sendMessage(this.message).queue(this :: consumeMessage);
		this.processed = true;
	}

	private void consumeReply(InteractionHook hook) {
		if(this.consumer == null) {
			return;
		}

		hook.retrieveOriginal().queue(this.consumer :: accept);
	}

	private void consumeMessage(Message message) {
		if(this.consumer == null) {
			return;
		}

		this.consumer.accept(message);
	}

	public static void start(IReplyCallback callback, long timeout, Consumer<Consumer<MessageCreateData>> action, Consumer<Message> consumer) {
		new TimeSafeReplyHandler(callback, timeout, action, consumer);
	}

	public static void start(IReplyCallback callback, Consumer<Consumer<MessageCreateData>> action) {
		TimeSafeReplyHandler.start(callback, 1500 /* Safety factor: 2x */, action, InteractionManager :: callback);
	}
}
