package com.khopan.hackontrol.service;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.eventlistener.FilteredEventListener;

import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.message.MessageReceivedEvent;

public class MessageService extends Service {
	@Override
	public void preBuild(JDABuilder builder) {
		builder.addEventListeners(FilteredEventListener.create(MessageReceivedEvent.class, Event -> {
			MessageChannelUnion channel = Event.getChannel();

			if("hackontrol_global_status".equalsIgnoreCase(Event.getMessage().getContentDisplay().trim())) {
				channel.sendMessage("**" + Hackontrol.getInstance().getMachineIdentifier() + ": Ok**").queue();
			}
		}));
	}
}
