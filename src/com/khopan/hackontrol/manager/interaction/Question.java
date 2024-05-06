package com.khopan.hackontrol.manager.interaction;

import java.util.function.Consumer;

import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.sendable.ISendable;

import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;

public class Question {
	private Question() {}

	public static void create(ISendable sender, String prompt, QuestionType type, Consumer<QuestionResponse> callback) {
		Question.custom(sender, prompt, type.positive, type.negative, callback);
	}

	public static void custom(ISendable sender, String prompt, String positiveResponse, String negativeResponse, Consumer<QuestionResponse> callback) {
		MessageCreateBuilder builder = new MessageCreateBuilder();
		builder.setContent("**" + HackontrolMessage.limit(prompt, 1996) + "**");
		builder.addActionRow(ButtonManager.dynamicButton(ButtonType.SUCCESS, positiveResponse, context -> Question.callback(context, callback, true)), ButtonManager.dynamicButton(ButtonType.DANGER, negativeResponse, context -> Question.callback(context, callback, false)));
		sender.send(builder.build(), InteractionManager :: callback);
	}

	public static void positive(ISendable sender, String prompt, String positiveResponse, String negativeResponse, Runnable onPositive) {
		Question.custom(sender, prompt, positiveResponse, negativeResponse, response -> {
			if(QuestionResponse.POSITIVE.equals(response)) {
				onPositive.run();
			}
		});
	}

	public static void positive(ISendable sender, String prompt, QuestionType type, Runnable onPositive) {
		Question.positive(sender, prompt, type.positive, type.negative, onPositive);
	}

	private static void callback(ButtonContext context, Consumer<QuestionResponse> callback, boolean response) {
		context.deferEdit().queue(hook -> hook.deleteOriginal().queue());
		callback.accept(response ? QuestionResponse.POSITIVE : QuestionResponse.NEGATIVE);
	}

	public static enum QuestionResponse {
		POSITIVE,
		NEGATIVE;
	}

	public static enum QuestionType {
		YES_NO("Yes", "No"),
		OK_CANCEL("Ok", "Cancel"),
		TRUE_FALSE("True", "False");

		private final String positive;
		private final String negative;

		QuestionType(String positive, String negative) {
			this.positive = positive;
			this.negative = negative;
		}
	}
}
