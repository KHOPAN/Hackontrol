package com.khopan.hackontrol.manager.interaction;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.registry.RegistrationHandler.RegistrationTypeEntry;

import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.events.interaction.ModalInteractionEvent;
import net.dv8tion.jda.api.interactions.modals.Modal;

public class ModalManager {
	private ModalManager() {}

	public static Modal.Builder dynamicModal(String title, Consumer<ModalContext> action, Object... parameters) {
		if(title == null) {
			title = "Modal";
		}

		long identifier = InteractionSession.newSession();
		Modal.Builder builder = ModalManager.staticModal(title, InteractionSession.prefix(identifier));
		InteractionSession session = new InteractionSession();
		session.sessionIdentifier = identifier;
		session.parameters = parameters;
		session.action = action;
		session.type = InteractionType.MODAL;
		InteractionSession.SESSION_LIST.add(session);
		return builder;
	}

	public static Modal.Builder staticModal(String title, String identifier) {
		return Modal.create(identifier, title);
	}

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
			InteractionSession.SESSION_LIST.remove(session);
		}

		if(action != null) {
			action.accept(new ModalContext(Event, session == null ? null : session.parameters));
		}
	}
}
