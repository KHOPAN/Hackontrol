package com.khopan.hackontrol.utils;

import java.io.BufferedReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.service.interaction.InteractionManager;
import com.khopan.hackontrol.utils.sendable.ISendable;
import com.khopan.hackontrol.utils.sendable.sender.ReplyCallbackSendable;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.requests.restaction.MessageCreateAction;
import net.dv8tion.jda.api.requests.restaction.interactions.ReplyCallbackAction;
import net.dv8tion.jda.api.utils.messages.MessageCreateRequest;

public class LargeMessage {
	private LargeMessage() {}

	public static void send(String largeMessage, IReplyCallback replyCallback, MessageConfigCallback configCallback) {
		List<String> messageList = LargeMessage.splitMessage(largeMessage, ReplyCallbackSendable.of(replyCallback));

		if(messageList == null) {
			return;
		}

		String firstMessage = messageList.get(0);
		messageList.remove(0);
		ReplyCallbackAction replyCallbackAction = replyCallback.reply(firstMessage);

		if(messageList.isEmpty()) {
			if(configCallback != null) {
				configCallback.configureMessage(replyCallbackAction, null);
			}

			replyCallbackAction.queue(InteractionManager :: callback);
			return;
		}

		MessageChannel channel = (MessageChannel) replyCallback.getChannel();
		replyCallbackAction.queue(hook -> {
			InteractionManager.callback(hook);
			hook.retrieveOriginal().queue(message -> new Thread(() -> {
				int size = messageList.size();
				MessageCreateAction messageCreateAction;

				if(size == 1) {
					messageCreateAction = channel.sendMessage(messageList.get(0));

					if(configCallback != null) {
						configCallback.configureMessage(messageCreateAction, new long[] {message.getIdLong()});
					}
				} else {
					long[] identifierList = new long[size];
					identifierList[0] = message.getIdLong();

					for(int i = 1; i < identifierList.length; i++) {
						identifierList[i] = channel.sendMessage(messageList.get(i - 1)).complete().getIdLong();
					}

					messageCreateAction = channel.sendMessage(messageList.get(size - 1));

					if(configCallback != null) {
						configCallback.configureMessage(messageCreateAction, identifierList);
					}
				}

				messageCreateAction.queue(InteractionManager :: callback);
			}).start());
		});
	}

	private static List<String> splitMessage(String message, ISendable reply) {
		BufferedReader reader = new BufferedReader(new StringReader(message));
		StringBuilder builder = new StringBuilder();
		List<String> messageList = new ArrayList<>();
		String line;

		try {
			while((line = reader.readLine()) != null) {
				line += '\n';

				if(builder.length() + line.length() > 2000) {
					messageList.add(builder.toString());
					builder = new StringBuilder();
				}

				builder.append(line);
			}

			if(!builder.isEmpty()) {
				messageList.add(builder.toString());
			}
		} catch(Throwable Errors) {
			HackontrolError.throwable(reply, Errors);
			return null;
		}

		return messageList;
	}

	public static interface MessageConfigCallback {
		void configureMessage(MessageCreateRequest<?> request, long[] messageIdentifiers);
	}
}
