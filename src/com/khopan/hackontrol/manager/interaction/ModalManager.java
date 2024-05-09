package com.khopan.hackontrol.manager.interaction;

import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.module.Module;
import com.khopan.hackontrol.utils.MultiConsumer;

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
	static void modalEvent(ModalInteractionEvent Event) {
		String identifier = Event.getModalId();
		InteractionSession session = InteractionSession.decodeSession(identifier);
		Hackontrol.LOGGER.info("Processing modal: {}", identifier);
		MultiConsumer<ModalContext> consumer = new MultiConsumer<>();

		if(session == null) {
			Module module = Hackontrol.getInstance().getModule((TextChannel) Event.getChannel());
			consumer.addAll(InteractionManager.MODAL_REGISTRY.filter(module, identifier));
		} else {
			if(!InteractionType.MODAL.equals(session.type)) {
				Hackontrol.LOGGER.warn("Mismatch modal interaction type: {}", identifier);
			}

			consumer.add((Consumer<ModalContext>) session.action);
			InteractionSession.SESSION_LIST.remove(session);
		}

		consumer.accept(new ModalContext(Event, session == null ? null : session.parameters));
	}
}
