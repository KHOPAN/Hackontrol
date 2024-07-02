package com.khopan.hackontrol.service.interaction;

import java.util.function.Consumer;

import com.khopan.hackontrol.manager.interaction.StringSelectContext;
import com.khopan.hackontrol.service.interaction.InteractionSession.InteractionType;

import net.dv8tion.jda.api.interactions.components.selections.StringSelectMenu;

public class StringSelectMenuManager {
	private StringSelectMenuManager() {}

	public static StringSelectMenu.Builder dynamicMenu(Consumer<StringSelectContext> action, Object... parameters) {
		long identifier = InteractionSession.newSession();
		StringSelectMenu.Builder builder = StringSelectMenuManager.staticMenu(InteractionSession.prefix(identifier));
		InteractionSession session = new InteractionSession();
		session.sessionIdentifier = identifier;
		session.parameters = parameters;
		session.action = action;
		session.type = InteractionType.STRING_SELECT_MENU;
		InteractionSession.SESSION_LIST.add(session);
		return builder;
	}

	public static StringSelectMenu.Builder staticMenu(String identifier) {
		return StringSelectMenu.create(identifier);
	}
}
