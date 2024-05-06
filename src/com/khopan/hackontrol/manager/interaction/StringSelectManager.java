package com.khopan.hackontrol.manager.interaction;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.registry.RegistrationHandler.RegistrationTypeEntry;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.events.interaction.component.StringSelectInteractionEvent;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.selections.StringSelectMenu;

public class StringSelectManager {
	private StringSelectManager() {}

	public static StringSelectMenu.Builder dynamicMenu(Consumer<StringSelectContext> action, Object... parameters) {
		long identifier = InteractionSession.newSession();
		StringSelectMenu.Builder builder = StringSelectManager.staticMenu(InteractionSession.prefix(identifier));
		InteractionSession session = new InteractionSession();
		session.sessionIdentifier = identifier;
		session.paramters = parameters;
		session.action = action;
		session.type = InteractionType.STRING_SELECT_MENU;
		InteractionSession.SESSION_LIST.add(session);
		return builder;
	}

	public static StringSelectMenu.Builder staticMenu(String identifier) {
		return StringSelectMenu.create(identifier);
	}

	@SuppressWarnings("unchecked")
	static void stringSelectInteractionEvent(StringSelectInteractionEvent Event, List<RegistrationTypeEntry<String, Consumer<StringSelectContext>>> stringSelectMenuList) {
		String identifier = Event.getInteraction().getSelectMenu().getId();
		InteractionSession session = InteractionSession.decodeSession(identifier);
		Hackontrol.LOGGER.info("Processing string select menu: {}", identifier);
		Consumer<StringSelectContext> action = null;

		if(session == null) {
			action = RegistrationTypeEntry.filter(stringSelectMenuList, Hackontrol.getInstance().getChannel((TextChannel) Event.getChannel()), identifier);
		} else {
			if(!InteractionType.STRING_SELECT_MENU.equals(session.type)) {
				Hackontrol.LOGGER.warn("Mismatch string select menu interaction type: {}", identifier);
			}

			action = (Consumer<StringSelectContext>) session.action;
		}

		if(action != null) {
			action.accept(new StringSelectContext(Event));
		}
	}

	static void assignIdentifier(Message message) {
		long messageIdentifier = message.getIdLong();
		List<ActionRow> rowList = message.getActionRows();

		for(int x = 0; x < rowList.size(); x++) {
			List<ItemComponent> componentList = rowList.get(x).getComponents();

			for(int y = 0; y < componentList.size(); y++) {
				if(componentList.get(y) instanceof StringSelectMenu menu) {
					InteractionSession session = InteractionSession.decodeSession(menu.getId());

					if(session != null) {
						session.messageIdentifier = messageIdentifier;
					}
				}
			}
		}
	}
}
