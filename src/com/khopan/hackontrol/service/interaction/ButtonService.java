package com.khopan.hackontrol.service.interaction;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.eventlistener.InteractionEventListener;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.module.Module;
import com.khopan.hackontrol.registry.TypeEntry;
import com.khopan.hackontrol.service.Service;
import com.khopan.hackontrol.service.interaction.InteractionSession.InteractionType;
import com.khopan.hackontrol.utils.MultiConsumer;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class ButtonService implements Service {
	@Override
	public void preBuild(JDABuilder builder) {
		builder.addEventListeners(InteractionEventListener.create(ButtonInteractionEvent.class, this :: buttonEvent));
	}

	@SuppressWarnings("unchecked")
	private void buttonEvent(ButtonInteractionEvent Event) {
		String identifier = Event.getButton().getId();
		long originalMessage = Event.getMessageIdLong();
		TextChannel channel = (TextChannel) Event.getChannel();

		if(HackontrolButton.deleteMessages(identifier, originalMessage, channel, Event.getMember())) {
			return;
		}

		InteractionSession session = InteractionSession.decodeSession(identifier);
		Hackontrol.LOGGER.info("Processing button: {}", identifier);
		MultiConsumer<ButtonContext> consumer = new MultiConsumer<>();

		if(session == null) {
			Module module = Hackontrol.getInstance().getModule(channel);
			List<TypeEntry<Button, Consumer<ButtonContext>>> buttonList = InteractionManager.BUTTON_REGISTRY.list(module);

			for(int i = 0; i < buttonList.size(); i++) {
				TypeEntry<Button, Consumer<ButtonContext>> entry = buttonList.get(i);

				if(identifier.equals(entry.getIdentifier().getId())) {
					consumer.add(entry.getValue());
				}
			}
		} else {
			if(!InteractionType.BUTTON.equals(session.type)) {
				Hackontrol.LOGGER.warn("Mismatch button interaction type: {}", identifier);
			}

			consumer.add((Consumer<ButtonContext>) session.action);
		}

		consumer.accept(new ButtonContext(Event, session == null ? null : session.parameters));
	}
}
