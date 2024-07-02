package com.khopan.hackontrol.service.interaction;

import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.eventlistener.InteractionEventListener;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.Service;
import com.khopan.hackontrol.service.interaction.InteractionSession.InteractionType;
import com.khopan.hackontrol.service.interaction.context.ButtonContext;
import com.khopan.hackontrol.utils.MultiConsumer;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class ButtonService extends Service {
	@SuppressWarnings("unchecked")
	@Override
	public void preBuild(JDABuilder builder) {
		builder.addEventListeners(InteractionEventListener.create(ButtonInteractionEvent.class, Event -> {
			String identifier = Event.getButton().getId();

			if(HackontrolButton.deleteMessages(identifier, Event.getMessageIdLong(), (TextChannel) Event.getChannel(), Event.getMember())) {
				return;
			}

			InteractionSession session = InteractionSession.decodeSession(identifier);
			MultiConsumer<ButtonContext> consumer = new MultiConsumer<>();

			if(session == null) {
				Hackontrol.LOGGER.info("Processing button: {}", identifier);
				this.panelManager.getRegistrable(Registration.BUTTON).forEach((key, value) -> {
					if(identifier.equals(key.getId())) {
						consumer.add(value);
					}
				});
			} else {
				if(!InteractionType.BUTTON.equals(session.type)) {
					Hackontrol.LOGGER.warn("Mismatch button interaction type: {}", identifier);
				}

				consumer.add((Consumer<ButtonContext>) session.action);
			}

			consumer.accept(new ButtonContext(Event, session == null ? null : session.parameters));
		}));
	}

	static void assignIdentifier(Message message) {
		long messageIdentifier = message.getIdLong();

		for(ActionRow row : message.getActionRows()) {
			for(Button button : row.getButtons()) {
				InteractionSession session = InteractionSession.decodeSession(button.getId());

				if(session != null) {
					session.messageIdentifier = messageIdentifier;
				}
			}
		}
	}
}
