package com.khopan.hackontrol.utils;

import java.io.PrintWriter;
import java.io.StringWriter;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.button.ButtonInteraction;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

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

	public static void sendErrorReply(ButtonInteraction interaction, Throwable Errors) {
		interaction.getEvent().reply("```" + ErrorUtils.getErrorMessage(Errors) + "```").addActionRow(Button.danger(Hackontrol.DELETE_SELF_IDENTIFIER, "Delete")).queue();
	}
}
