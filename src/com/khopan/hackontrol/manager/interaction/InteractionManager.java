package com.khopan.hackontrol.manager.interaction;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.eventlistener.FilteredEventListener;
import com.khopan.hackontrol.eventlistener.InteractionEventListener;
import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.registry.RegistryType;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.events.interaction.ModalInteractionEvent;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.events.interaction.component.StringSelectInteractionEvent;
import net.dv8tion.jda.api.events.message.MessageDeleteEvent;
import net.dv8tion.jda.api.interactions.InteractionHook;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class InteractionManager implements Manager {
	public static final RegistryType<Button, Consumer<ButtonContext>> BUTTON_REGISTRY = RegistryType.create();
	public static final RegistryType<String, Consumer<ModalContext>> MODAL_REGISTRY = RegistryType.create();
	public static final RegistryType<String, Consumer<StringSelectContext>> STRING_SELECT_MENU_REGISTRY = RegistryType.create();

	@Override
	public void configureBuilder(JDABuilder builder) {
		builder.addEventListeners(InteractionEventListener.create(ButtonInteractionEvent.class, Event -> ButtonManager.buttonEvent(Event)));
		builder.addEventListeners(InteractionEventListener.create(ModalInteractionEvent.class, Event -> ModalManager.modalEvent(Event)));
		builder.addEventListeners(InteractionEventListener.create(StringSelectInteractionEvent.class, Event -> StringSelectManager.stringSelectInteractionEvent(Event)));
		builder.addEventListeners(FilteredEventListener.create(MessageDeleteEvent.class, this :: deleteEvent));
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
		StringSelectManager.assignIdentifier(message);
	}

	public static void callback(InteractionHook hook) {
		hook.retrieveOriginal().queue(InteractionManager :: callback);
	}
}
