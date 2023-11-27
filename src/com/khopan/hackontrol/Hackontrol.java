package com.khopan.hackontrol;

import com.khopan.hackontrol.command.Command;
import com.khopan.hackontrol.command.DeviceListCommand;
import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.exceptions.CommandSyntaxException;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.message.MessageReceivedEvent;
import net.dv8tion.jda.api.hooks.ListenerAdapter;
import net.dv8tion.jda.api.requests.GatewayIntent;

public class Hackontrol {
	private final long userIdentifier;
	private final CommandDispatcher<CommandSource> dispatcher;

	private Hackontrol(JDA bot) {
		bot.addEventListener(new Listener());
		this.userIdentifier = bot.getSelfUser().getIdLong();
		this.dispatcher = new CommandDispatcher<>();
		this.registerCommand(new DeviceListCommand());
	}

	private void registerCommand(Command command) {
		command.register(this.dispatcher);
	}

	private void processMessage(MessageReceivedEvent Event) {
		if(Event.getAuthor().getIdLong() == this.userIdentifier) {
			return;
		}

		MessageChannelUnion channel = Event.getChannel();
		Message message = Event.getMessage();
		String content = message.getContentDisplay();

		if(content == null || content.isBlank()) {
			return;
		}

		if(content.indexOf('\n') == -1) {
			this.processSingleMessage(content, channel);
			return;
		}

		String[] parts = content.split("\n");

		if(parts == null || parts.length == 0) {
			return;
		}

		for(int i = 0; i < parts.length; i++) {
			String part = parts[i];

			if(part == null || part.isBlank()) {
				continue;
			}

			this.processSingleMessage(part, channel);
		}
	}

	private void processSingleMessage(String message, MessageChannel channel) {
		message = message.trim();

		if(!message.startsWith("$")) {
			return;
		}

		message = message.substring(1);

		if(message.isBlank()) {
			return;
		}

		System.out.println("Command: " + message);

		try {
			this.dispatcher.execute(message, null);
		} catch(CommandSyntaxException Exception) {
			channel.sendMessage('`' + Exception.getMessage() + '`').queue();
		}
	}

	public static void main(String[] args) {
		JDA bot = JDABuilder.createDefault(Token.BOT_TOKEN)
				.enableIntents(GatewayIntent.MESSAGE_CONTENT)
				.build();

		try {
			bot.awaitReady();
		} catch(Throwable ignored) {
			return;
		}

		new Hackontrol(bot);
	}

	private class Listener extends ListenerAdapter {
		@Override
		public void onMessageReceived(MessageReceivedEvent Event) {
			Hackontrol.this.processMessage(Event);
		}
	}
}
