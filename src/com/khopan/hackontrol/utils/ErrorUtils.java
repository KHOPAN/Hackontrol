package com.khopan.hackontrol.utils;

import java.io.PrintWriter;
import java.io.StringWriter;

import com.khopan.hackontrol.manager.button.ButtonContext;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.manager.common.IReplyHandler;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;

public class ErrorUtils {
	private ErrorUtils() {}

	public static String getErrorMessage(Throwable Errors) {
		StringWriter stringWriter = new StringWriter();
		PrintWriter printWriter = new PrintWriter(stringWriter);
		Errors.printStackTrace(printWriter);
		String text = stringWriter.toString();
		printWriter.close();
		return text;
	}

	public static void sendErrorReply(IReplyHandler handler, Throwable Errors) {
		String message = ErrorUtils.limitBlock(ErrorUtils.getErrorMessage(Errors));
		handler.reply(message).addActionRow(ButtonManager.selfDelete(ButtonStyle.DANGER, "Delete")).queue();
	}

	public static void sendErrorMessage(MessageChannel channel, Throwable Errors) {
		String message = ErrorUtils.limitBlock(ErrorUtils.getErrorMessage(Errors));
		channel.sendMessage(message).addActionRow(ButtonManager.selfDelete(ButtonStyle.DANGER, "Delete")).queue();
	}

	public static void sendErrorMessage(ButtonContext context, Throwable Errors) {
		ErrorUtils.sendErrorMessage(context.getChannel(), Errors);
	}

	private static String limitBlock(String text) {
		String message = ErrorUtils.limit(text, 1992);
		return "```\n" + message + "\n```";
	}

	private static String limit(String text, int limit) {
		if(text.length() > limit) {
			text = text.substring(0, limit);
		}

		return text;
	}
}
