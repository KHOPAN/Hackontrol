package com.khopan.hackontrol.utils.sendable;

import java.util.function.Consumer;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public interface ISendable {
	void send(MessageCreateData message, Consumer<Message> callback);
}
