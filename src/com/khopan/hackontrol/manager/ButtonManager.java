package com.khopan.hackontrol.manager;

import com.khopan.hackontrol.eventlistener.FilteredEventListener;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;

public class ButtonManager implements Manager {
	public ButtonManager() {

	}

	@Override
	public void configureBuilder(JDABuilder builder) {
		builder.addEventListeners(FilteredEventListener.create(ButtonInteractionEvent.class, this :: buttonCallback));
	}

	private void buttonCallback(ButtonInteractionEvent Event) {
		System.out.println("Callback");
	}
}
