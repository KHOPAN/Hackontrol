package com.khopan.hackontrol.utils.interaction;

import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;

import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class HackontrolButton {
	private HackontrolButton() {}

	public static Button delete(Object... parameters) {
		return ButtonManager.dynamicButton(ButtonType.DANGER, "Delete", HackontrolButton :: deleteCallback, parameters);
	}

	private static void deleteCallback(ButtonContext context) {
		HackontrolButton.deleteMessagesInParameters(context);
		context.deferEdit().queue(hook -> hook.deleteOriginal().queue());
	}

	public static void deleteMessagesInParameters(ButtonContext context) {
		if(context.hasParameter()) {
			Object[] parameters = context.getParameters();
			MessageChannelUnion channel = context.getChannel();

			for(int i = 0; i < parameters.length; i++) {
				if(parameters[i] instanceof Number number) {
					channel.retrieveMessageById(number.longValue()).queue(message -> message.delete().queue(), Errors -> {});
				}
			}
		}
	}
}
