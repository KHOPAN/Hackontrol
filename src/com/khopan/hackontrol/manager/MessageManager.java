package com.khopan.hackontrol.manager;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.eventlistener.FilteredEventListener;
import com.khopan.hackontrol.registry.RegistryType;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.message.MessageReceivedEvent;

public class MessageManager implements Manager {
	public static final RegistryType<Object, Consumer<MessageReceivedEvent>> MESSAGE_RECEIVED_EVENT_REGISTRY = RegistryType.create();

	@Override
	public void configureBuilder(JDABuilder builder) {
		builder.addEventListeners(FilteredEventListener.create(MessageReceivedEvent.class, this :: receivedEvent));
	}

	private void receivedEvent(MessageReceivedEvent Event) {
		MessageChannelUnion channel = Event.getChannel();

		if("hackontrol_global_status".equalsIgnoreCase(Event.getMessage().getContentDisplay().trim())) {
			channel.sendMessage("**" + Hackontrol.getInstance().getMachineIdentifier() + ": Ok**").queue();
		}

		if(!HackontrolMessage.checkCategory(channel)) {
			return;
		}

		HackontrolChannel hackontrolChannel = Hackontrol.getInstance().getChannel((TextChannel) channel);
		List<Consumer<MessageReceivedEvent>> list = MessageManager.MESSAGE_RECEIVED_EVENT_REGISTRY.filter(hackontrolChannel, null);

		for(int i = 0; i < list.size(); i++) {
			Consumer<MessageReceivedEvent> action = list.get(i);

			if(action != null) {
				action.accept(Event);
			}
		}
	}
}
