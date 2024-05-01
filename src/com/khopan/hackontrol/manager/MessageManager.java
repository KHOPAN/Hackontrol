package com.khopan.hackontrol.manager;

import java.util.List;
import java.util.function.Consumer;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.eventlistener.FilteredEventListener;
import com.khopan.hackontrol.registry.RegistrationHandler;
import com.khopan.hackontrol.registry.RegistrationHandler.RegistrationTypeEntry;
import com.khopan.hackontrol.registry.RegistryType;
import com.khopan.hackontrol.utils.DiscordUtils;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.message.MessageReceivedEvent;

public class MessageManager implements Manager {
	public static final RegistryType<Object, Consumer<MessageReceivedEvent>> MESSAGE_RECEIVED_EVENT_REGISTRY = RegistryType.create();

	private List<RegistrationTypeEntry<Object, Consumer<MessageReceivedEvent>>> messageReceivedEventList;

	@Override
	public void configureBuilder(JDABuilder builder) {
		builder.addEventListeners(FilteredEventListener.create(MessageReceivedEvent.class, this :: receivedEvent));
	}

	@Override
	public void initialize(RegistrationHandler handler) {
		this.messageReceivedEventList = handler.filterType(MessageManager.MESSAGE_RECEIVED_EVENT_REGISTRY);
	}

	private void receivedEvent(MessageReceivedEvent Event) {
		MessageChannelUnion channel = Event.getChannel();

		if(!DiscordUtils.checkCategory(channel)) {
			return;
		}

		HackontrolChannel hackontrolChannel = Hackontrol.getInstance().getChannel((TextChannel) channel);
		Consumer<MessageReceivedEvent> action = RegistrationTypeEntry.filter(this.messageReceivedEventList, hackontrolChannel, null);

		if(action != null) {
			action.accept(Event);
		}
	}
}
