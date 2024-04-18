package com.khopan.hackontrol;

import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;

public interface ButtonInteraction {
	ButtonInteractionEvent getEvent();
	void consume();
}
