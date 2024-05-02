package com.khopan.hackontrol.utils;

import java.io.PrintWriter;
import java.io.StringWriter;

import com.khopan.hackontrol.manager.common.sender.sendable.ISendable;

public class HackontrolError {
	private HackontrolError() {}

	public static String getMessage(Throwable Errors) {
		if(Errors == null) {
			throw new NullPointerException("Errors cannot be null");
		}

		StringWriter stringWriter = new StringWriter();
		PrintWriter printWriter = new PrintWriter(stringWriter);
		Errors.printStackTrace(printWriter);
		String text = stringWriter.toString();
		printWriter.close();
		return text == null ? "" : text;
	}

	public static void throwable(ISendable sender, Throwable Errors) {
		if(Errors == null) {
			throw new NullPointerException("Errors cannot be null");
		}

		HackontrolMessage.codeblockDeletable(sender, HackontrolError.getMessage(Errors));
	}

	public static void message(ISendable sender, String message) {
		if(sender == null) {
			throw new NullPointerException("Sender cannot be null");
		}

		if(message == null) {
			throw new NullPointerException("Message cannot be null");
		}

		message = HackontrolMessage.limit(message, 1991);
		HackontrolMessage.deletableInternal(sender, "`Error: " + message + '`');
	}

	public static void multiline(ISendable sender, String message) {
		if(sender == null) {
			throw new NullPointerException("Sender cannot be null");
		}

		if(message == null) {
			throw new NullPointerException("Message cannot be null");
		}

		int lines = HackontrolError.count(message, '\n') + 1;
		int limit = 2000 - lines * 2;
		message = HackontrolMessage.limit(message, limit);
		StringBuilder builder = new StringBuilder();
		String[] parts = message.split("\n");

		for(int i = 0; i < parts.length; i++) {
			if(i > 0) {
				builder.append('\n');
			}

			builder.append('`');
			builder.append(parts[i]);
			builder.append('`');
		}

		HackontrolMessage.deletableInternal(sender, builder.toString());
	}

	private static int count(String text, char character) {
		int count = 0;

		for(int i = 0; i < text.length(); i++) {
			if(text.charAt(i) == character) {
				count++;
			}
		}

		return count;
	}
}
