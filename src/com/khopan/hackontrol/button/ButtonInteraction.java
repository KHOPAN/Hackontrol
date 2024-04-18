package com.khopan.hackontrol.button;

import java.util.function.Consumer;

import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;

public interface ButtonInteraction {
	ButtonInteractionEvent getEvent();
	void consume();
	void okCancelQuestion(String question, Consumer<Boolean> callback);
	void yesNoQuestion(String question, Consumer<Boolean> callback);
}
