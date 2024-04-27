package com.khopan.hackontrol.manager.button;

import java.util.function.Consumer;

import com.khopan.hackontrol.manager.common.sender.sendable.ISendable;

import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;
import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;

public class Question {
	private Question() {}

	public static void create(ISendable sender, String prompt, OptionType option, Consumer<QuestionResponse> callback) {
		Question.custom(sender, prompt, option.positive, option.negative, callback);
	}

	public static void custom(ISendable sender, String prompt, String positiveResponse, String negativeResponse, Consumer<QuestionResponse> callback) {
		MessageCreateBuilder builder = new MessageCreateBuilder();
		builder.setContent(prompt);
		builder.addActionRow(ButtonManager.dynamicButton(ButtonStyle.SUCCESS, positiveResponse, context -> Question.callback(context, callback, true)), ButtonManager.dynamicButton(ButtonStyle.DANGER, negativeResponse, context -> Question.callback(context, callback, false)));
		sender.send(builder.build(), ButtonManager :: dynamicButtonCallback);
	}

	private static void callback(ButtonContext context, Consumer<QuestionResponse> callback, boolean response) {
		context.delete();
		callback.accept(response ? QuestionResponse.POSITIVE_RESPONSE : QuestionResponse.NEGATIVE_RESPONSE);
	}

	public static enum QuestionResponse {
		POSITIVE_RESPONSE,
		NEGATIVE_RESPONSE;
	}

	public static enum OptionType {
		YES_NO("Yes", "No"),
		OK_CANCEL("Ok", "Cancel"),
		TRUE_FALSE("True", "False");

		private final String positive;
		private final String negative;

		OptionType(String positive, String negative) {
			this.positive = positive;
			this.negative = negative;
		}
	}
}
