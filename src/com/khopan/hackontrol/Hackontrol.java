package com.khopan.hackontrol;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.events.message.MessageReceivedEvent;
import net.dv8tion.jda.api.hooks.ListenerAdapter;
import net.dv8tion.jda.api.requests.GatewayIntent;

public class Hackontrol {
	private final long userIdentifier;

	private Hackontrol(JDA bot) {
		bot.addEventListener(new Listener());
		this.userIdentifier = bot.getSelfUser().getIdLong();
	}

	private void processMessage(MessageReceivedEvent Event) {
		if(Event.getAuthor().getIdLong() == this.userIdentifier) {
			return;
		}

		Message message = Event.getMessage();
		String content = message.getContentDisplay();

		if(content == null || content.isBlank()) {
			return;
		}

		if(content.indexOf('\n') == -1) {
			this.processSingleMessage(content);
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

			this.processSingleMessage(part);
		}
	}

	private void processSingleMessage(String message) {
		if(!message.startsWith("$")) {
			return;
		}

		System.out.println("Command: " + message);
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
