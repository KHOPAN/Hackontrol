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
		HackontrolMessage.codeblockDeletable(sender, message);
	}
}