package com.khopan.hackontrol.service.interaction;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.Service;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.event.FilteredEventListener;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.events.message.MessageDeleteEvent;
import net.dv8tion.jda.api.events.message.MessageReceivedEvent;

public class MessageService extends Service {
	@Override
	public void preBuild(JDABuilder builder) {
		builder.addEventListeners(FilteredEventListener.create(MessageReceivedEvent.class, Event -> {
			for(Consumer<MessageReceivedEvent> consumer : this.panelManager.getRegistrable(Registration.MESSAGE_RECEIVED_EVENT)) {
				if(consumer != null) {
					consumer.accept(Event);
				}
			}
		}));

		builder.addEventListeners(FilteredEventListener.create(MessageDeleteEvent.class, Event -> {
			if(!HackontrolMessage.checkCategory(Event.getChannel())) {
				return;
			}

			long messageIdentifier = Event.getMessageIdLong();
			List<InteractionSession> deleteList = new ArrayList<>();

			for(InteractionSession session : InteractionSession.SESSION_LIST) {
				if(session.messageIdentifier == messageIdentifier) {
					deleteList.add(session);
				}
			}

			InteractionSession.SESSION_LIST.removeAll(deleteList);
		}));
	}
}
