package com.khopan.hackontrol.utils.sendable.sender;

import java.util.function.Consumer;

import com.khopan.hackontrol.utils.sendable.ISendable;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class ReplyCallbackSendable implements ISendable {
	private final IReplyCallback callback;

	private ReplyCallbackSendable(IReplyCallback callback) {
		this.callback = callback;
	}

	@Override
	public void send(MessageCreateData message, Consumer<Message> callback) {
		this.callback.reply(message).queue(hook -> hook.retrieveOriginal().queue(callback));
	}

	public static ISendable of(IReplyCallback callback) {
		return new ReplyCallbackSendable(callback);
	}
}
