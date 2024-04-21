package com.khopan.hackontrol.manager;

import java.util.function.Consumer;

import com.khopan.hackontrol.button.ButtonInteraction;
import com.khopan.hackontrol.eventlistener.FilteredEventListener;
import com.khopan.hackontrol.registry.RegistryType;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;

public class ButtonManager implements Manager {
	public static final RegistryType<String, Consumer<ButtonInteraction>> BUTTON_CALLBACK_REGISTRY = RegistryType.create();

	@Override
	public void configureBuilder(JDABuilder builder) {
		builder.addEventListeners(FilteredEventListener.create(ButtonInteractionEvent.class, this :: buttonEvent));
	}

	private void buttonEvent(ButtonInteractionEvent Event) {
		System.out.println("Callback");
	}
}
