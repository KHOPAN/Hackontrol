package com.khopan.hackontrol.utils;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.ISendable;

import net.dv8tion.jda.api.entities.channel.Channel;
import net.dv8tion.jda.api.entities.channel.attribute.ICategorizableChannel;
import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.interactions.components.ComponentInteraction;
import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;

public class HackontrolMessage {
	private HackontrolMessage() {}

	public static void deletable(ISendable sender, String message) {
		if(message == null) {
			throw new NullPointerException("Message cannot be null");
		}

		HackontrolMessage.deletableInternal(sender, HackontrolMessage.limit(message, 2000));
	}

	public static void boldDeletable(ISendable sender, String message) {
		if(message == null) {
			throw new NullPointerException("Message cannot be null");
		}

		HackontrolMessage.deletableInternal(sender, "**" + HackontrolMessage.limit(message, 1996) + "**");
	}

	public static void codeblockDeletable(ISendable sender, String message) {
		if(message == null) {
			throw new NullPointerException("Message cannot be null");
		}

		HackontrolMessage.deletableInternal(sender, HackontrolMessage.limitBlock(message));
	}

	public static void delete(ComponentInteraction interaction) {
		interaction.getChannel().deleteMessageById(interaction.getMessageIdLong()).queue();
	}

	static void deletableInternal(ISendable sender, String message) {
		if(sender == null) {
			throw new NullPointerException("Sender cannot be null");
		}

		MessageCreateBuilder builder = new MessageCreateBuilder();
		builder.setContent(message);
		builder.addActionRow(HackontrolButton.delete());
		sender.send(builder.build(), InteractionManager :: callback);
	}

	private static String limitBlock(String text) {
		return "```\n" + HackontrolMessage.limit(text, 1992) + "\n```";
	}

	public static String limit(String text, int limit) {
		if(!text.isEmpty() && text.length() > limit) {
			text = text.substring(0, limit);
		}

		return text;
	}

	public static boolean checkCategory(Channel channel) {
		if(!(channel instanceof ICategorizableChannel)) {
			return false;
		}

		Category category = ((ICategorizableChannel) channel).getParentCategory();
		Hackontrol hackontrol = Hackontrol.getInstance();
		return hackontrol.getCategory().equals(category);
	}
}
