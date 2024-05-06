package com.khopan.hackontrol.manager.interaction;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.registry.RegistrationHandler.RegistrationTypeEntry;

import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.events.interaction.ModalInteractionEvent;

public class ModalInteractionManager {
	ModalInteractionManager() {}

	@SuppressWarnings("unchecked")
	static void modalEvent(ModalInteractionEvent Event, List<RegistrationTypeEntry<String, Consumer<ModalContext>>> modalList) {
		String identifier = Event.getModalId();
		InteractionSession session = InteractionSession.decodeSession(identifier);
		Hackontrol.LOGGER.info("Processing modal: {}", identifier);
		Consumer<ModalContext> action = null;

		if(session == null) {
			action = RegistrationTypeEntry.filter(modalList, Hackontrol.getInstance().getChannel((TextChannel) Event.getChannel()), identifier);
		} else {
			if(!InteractionType.MODAL.equals(session.type)) {
				Hackontrol.LOGGER.warn("Mismatch modal interaction type: {}", identifier);
			}

			action = (Consumer<ModalContext>) session.action;
		}

		if(action != null) {
			action.accept(new ModalContext(Event, session == null ? null : session.paramters));
		}
	}
}
