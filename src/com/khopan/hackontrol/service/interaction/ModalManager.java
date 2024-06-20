package com.khopan.hackontrol.service.interaction;

import java.util.function.Consumer;

import com.khopan.hackontrol.manager.interaction.ModalContext;
import com.khopan.hackontrol.service.interaction.InteractionSession.InteractionType;

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
}
