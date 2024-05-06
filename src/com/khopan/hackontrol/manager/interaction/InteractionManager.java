package com.khopan.hackontrol.manager.interaction;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.eventlistener.FilteredEventListener;
import com.khopan.hackontrol.eventlistener.InteractionEventListener;
import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.registry.RegistrationHandler;
import com.khopan.hackontrol.registry.RegistrationHandler.RegistrationTypeEntry;
import com.khopan.hackontrol.registry.RegistryType;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.events.interaction.ModalInteractionEvent;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.events.message.MessageDeleteEvent;
import net.dv8tion.jda.api.interactions.InteractionHook;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class InteractionManager implements Manager {
	public static final RegistryType<Button, Consumer<ButtonContext>> BUTTON_REGISTRY = RegistryType.create();
	public static final RegistryType<String, Consumer<ModalContext>> MODAL_REGISTRY = RegistryType.create();

	private List<RegistrationTypeEntry<Button, Consumer<ButtonContext>>> buttonList;
	private List<RegistrationTypeEntry<String, Consumer<ModalContext>>> modalList;

	@Override
	public void configureBuilder(JDABuilder builder) {
		builder.addEventListeners(InteractionEventListener.create(ButtonInteractionEvent.class, Event -> ButtonManager.buttonEvent(Event, this.buttonList)));
		builder.addEventListeners(InteractionEventListener.create(ModalInteractionEvent.class, Event -> ModalInteractionManager.modalEvent(Event, this.modalList)));
		builder.addEventListeners(FilteredEventListener.create(MessageDeleteEvent.class, this :: deleteEvent));
	}

	@Override
	public void initialize(RegistrationHandler handler) {
		this.buttonList = handler.filterType(InteractionManager.BUTTON_REGISTRY);
		this.modalList = handler.filterType(InteractionManager.MODAL_REGISTRY);
	}

	private void deleteEvent(MessageDeleteEvent Event) {
		if(!HackontrolMessage.checkCategory(Event.getChannel())) {
			return;
		}

		long messageIdentifier = Event.getMessageIdLong();
		List<InteractionSession> deleteList = new ArrayList<>();

		for(int i = 0; i < InteractionSession.SESSION_LIST.size(); i++) {
			InteractionSession session = InteractionSession.SESSION_LIST.get(i);

			if(session.messageIdentifier == messageIdentifier) {
				deleteList.add(session);
			}
		}

		InteractionSession.SESSION_LIST.removeAll(deleteList);
	}

	public static void callback(Message message) {
		ButtonManager.assignIdentifier(message);
	}

	public static void callback(InteractionHook hook) {
		hook.retrieveOriginal().queue(InteractionManager :: callback);
	}
}
