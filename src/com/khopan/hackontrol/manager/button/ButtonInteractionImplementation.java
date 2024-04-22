package com.khopan.hackontrol.manager.button;

import java.util.function.Consumer;

import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;

public class ButtonInteractionImplementation implements ButtonInteraction {
	private final ButtonInteractionEvent Event;
	private final Object[] parameters;

	public ButtonInteractionImplementation(ButtonInteractionEvent Event, Object[] parameters) {
		this.Event = Event;
		this.parameters = parameters;
	}

	@Override
	public ButtonInteractionEvent getEvent() {
		return this.Event;
	}

	@Override
	public Object[] getParameters() {
		return this.parameters;
	}

	@Override
	public void delete() {
		this.Event.deferEdit().queue(hook -> hook.deleteOriginal().queue());
	}

	@Override
	public void consume() {
		this.Event.deferEdit().queue();
	}

	@Override
	public void okCancelQuestion(String question, Consumer<Boolean> callback) {
		this.question(question, callback, "Ok", "Cancel");
	}

	@Override
	public void yesNoQuestion(String question, Consumer<Boolean> callback) {
		this.question(question, callback, "Yes", "No");
	}

	private void question(String question, Consumer<Boolean> callback, String ok, String cancel) {
		this.Event.reply(question).addActionRow(
				ButtonManager.dynamicButton(ButtonStyle.SUCCESS, ok, interaction -> this.questionCallback(interaction, callback, true)),
				ButtonManager.dynamicButton(ButtonStyle.DANGER, cancel, interaction -> this.questionCallback(interaction, callback, false))
				).queue(ButtonManager :: dynamicButtonCallback);
	}

	private void questionCallback(ButtonInteraction interaction, Consumer<Boolean> callback, boolean response) {
		interaction.delete();
		callback.accept(response);
	}
}
