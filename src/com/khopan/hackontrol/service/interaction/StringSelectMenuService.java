package com.khopan.hackontrol.service.interaction;

import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.eventlistener.InteractionEventListener;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.Service;
import com.khopan.hackontrol.service.interaction.InteractionSession.InteractionType;
import com.khopan.hackontrol.service.interaction.context.StringSelectContext;
import com.khopan.hackontrol.utils.MultiConsumer;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.events.interaction.component.StringSelectInteractionEvent;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.selections.StringSelectMenu;

public class StringSelectMenuService extends Service {
	@SuppressWarnings("unchecked")
	@Override
	public void preBuild(JDABuilder builder) {
		builder.addEventListeners(InteractionEventListener.create(StringSelectInteractionEvent.class, Event -> {
			String identifier = Event.getInteraction().getSelectMenu().getId();
			InteractionSession session = InteractionSession.decodeSession(identifier);
			MultiConsumer<StringSelectContext> consumer = new MultiConsumer<>();

			if(session == null) {
				Hackontrol.LOGGER.info("Processing string select menu: {}", identifier);
				this.panelManager.getRegistrable(Registration.STRING_SELECT_MENU).forEach((key, value) -> {
					if(identifier.equals(key)) {
						consumer.add(value);
					}
				});
			} else {
				if(!InteractionType.STRING_SELECT_MENU.equals(session.type)) {
					Hackontrol.LOGGER.warn("Mismatch string select menu interaction type: {}", identifier);
				}

				consumer.add((Consumer<StringSelectContext>) session.action);
			}

			consumer.accept(new StringSelectContext(Event, session == null ? null : session.parameters));
		}));
	}

	static void assignIdentifier(Message message) {
		long messageIdentifier = message.getIdLong();

		for(ActionRow row : message.getActionRows()) {
			for(ItemComponent component : row.getComponents()) {
				if(!(component instanceof StringSelectMenu)) {
					continue;
				}

				InteractionSession session = InteractionSession.decodeSession(((StringSelectMenu) component).getId());

				if(session != null) {
					session.messageIdentifier = messageIdentifier;
				}
			}
		}
	}
}
