package com.khopan.hackontrol.manager.common.sender.sendable;

import java.util.function.Consumer;

import com.khopan.hackontrol.manager.common.sender.IRepliable;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class ReplySendable implements ISendable {
	private final IRepliable repliable;

	private ReplySendable(IRepliable repliable) {
		this.repliable = repliable;
	}

	@Override
	public void send(MessageCreateData message, Consumer<Message> callback) {
		this.repliable.reply(message).queue(hook -> hook.retrieveOriginal().queue(callback));
	}

	public static ISendable of(IRepliable repliable) {
		return new ReplySendable(repliable);
	}
}
