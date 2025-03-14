package com.khopan.hackontrol.service.interaction;

import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.Service;
import com.khopan.hackontrol.service.interaction.InteractionSession.InteractionType;
import com.khopan.hackontrol.service.interaction.context.ModalContext;
import com.khopan.hackontrol.utils.MultiConsumer;
import com.khopan.hackontrol.utils.event.InteractionEventListener;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.events.interaction.ModalInteractionEvent;

public class ModalService extends Service {
	@SuppressWarnings("unchecked")
	@Override
	public void preBuild(JDABuilder builder) {
		builder.addEventListeners(InteractionEventListener.create(ModalInteractionEvent.class, Event -> {
			String identifier = Event.getModalId();
			InteractionSession session = InteractionSession.decodeSession(identifier);
			MultiConsumer<ModalContext> consumer = new MultiConsumer<>();

			if(session == null) {
				Hackontrol.LOGGER.info("Processing modal: {}", identifier);
				this.panelManager.getRegistrable(Registration.MODAL).forEach((key, value) -> {
					if(identifier.equals(key)) {
						consumer.add(value);
					}
				});
			} else {
				if(!InteractionType.MODAL.equals(session.type)) {
					Hackontrol.LOGGER.warn("Mismatch modal interaction type: {}", identifier);
				}

				consumer.add((Consumer<ModalContext>) session.action);
				InteractionSession.SESSION_LIST.remove(session);
			}

			consumer.accept(new ModalContext(Event, session == null ? null : session.parameters));
		}));
	}
}
