package com.khopan.hackontrol.manager.modal;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.eventlistener.InteractionEventListener;
import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.registry.RegistrationHandler;
import com.khopan.hackontrol.registry.RegistrationHandler.RegistrationTypeEntry;
import com.khopan.hackontrol.registry.RegistryType;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.events.interaction.ModalInteractionEvent;

public class ModalManager implements Manager {
	public static final RegistryType<String, Consumer<ModalContext>> MODAL_REGISTRY = RegistryType.create();

	private List<RegistrationTypeEntry<String, Consumer<ModalContext>>> modalList;

	@Override
	public void configureBuilder(JDABuilder builder) {
		builder.addEventListeners(InteractionEventListener.create(ModalInteractionEvent.class, this :: modalEvent));
	}

	@Override
	public void initialize(RegistrationHandler handler) {
		this.modalList = handler.filterType(ModalManager.MODAL_REGISTRY);
	}

	private void modalEvent(ModalInteractionEvent Event) {
		String identifier = Event.getModalId();
		Hackontrol.LOGGER.info("Processing modal: {}", identifier);
		Consumer<ModalContext> action = RegistrationTypeEntry.filter(this.modalList, Hackontrol.getInstance().getChannel((TextChannel) Event.getChannel()), identifier);

		if(action == null) {
			Hackontrol.LOGGER.warn("Modal {} has null action", identifier);
			return;
		}

		action.accept(new ModalContextImplementation(Event));
	}
}
