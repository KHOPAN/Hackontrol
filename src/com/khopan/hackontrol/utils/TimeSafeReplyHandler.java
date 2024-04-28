package com.khopan.hackontrol.utils;

import java.util.function.Consumer;

import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.manager.common.IThinkable;
import com.khopan.hackontrol.manager.common.sender.IRepliable;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.interactions.InteractionHook;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class TimeSafeReplyHandler {
	private static int Count;

	private final IRepliable repliable;
	private final Consumer<Message> consumer;

	private volatile MessageCreateData message;
	private volatile InteractionHook hook;
	private volatile boolean processed;

	private boolean defer;

	private TimeSafeReplyHandler(IThinkable thinkable, IRepliable repliable, long timeout, Consumer<Consumer<MessageCreateData>> action, Consumer<Message> consumer) {
		long start = System.currentTimeMillis();
		this.repliable = repliable;
		this.consumer = consumer;
		Thread thread = new Thread(() -> action.accept(this :: callback));
		thread.setName("Hackontrol Time-Safe Reply Handler #" + (++TimeSafeReplyHandler.Count));
		thread.start();

		try {
			Thread.sleep(timeout - (System.currentTimeMillis() - start));
		} catch(Throwable Errors) {
			Errors.printStackTrace();
		}

		if(this.message == null) {
			this.defer = true;
			thinkable.thinking(this :: hook);
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
			this.repliable.reply(this.message).queue(this :: consumeReply);
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

	public static void start(IThinkable thinkable, IRepliable repliable, long timeout, Consumer<Consumer<MessageCreateData>> action, Consumer<Message> consumer) {
		new TimeSafeReplyHandler(thinkable, repliable, timeout, action, consumer);
	}

	public static <T extends IThinkable & IRepliable> void start(T thinkableRepliable, long timeout, Consumer<Consumer<MessageCreateData>> action, Consumer<Message> consumer) {
		TimeSafeReplyHandler.start(thinkableRepliable, thinkableRepliable, timeout, action, consumer);
	}

	public static void start(IThinkable thinkable, IRepliable repliable, Consumer<Consumer<MessageCreateData>> action) {
		TimeSafeReplyHandler.start(thinkable, repliable, 1500 /* Safety factor: 2x */, action, ButtonManager :: dynamicButtonCallback);
	}

	public static <T extends IThinkable & IRepliable> void start(T thinkableRepliable, Consumer<Consumer<MessageCreateData>> action) {
		TimeSafeReplyHandler.start(thinkableRepliable, thinkableRepliable, action);
	}
}
