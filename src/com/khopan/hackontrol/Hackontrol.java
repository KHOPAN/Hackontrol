package com.khopan.hackontrol;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;

public class Hackontrol {
	private Hackontrol(JDA bot) {

	}

	public static void main(String[] args) {
		JDA bot = JDABuilder.createDefault(Token.BOT_TOKEN)
				.build();

		try {
			bot.awaitReady();
		} catch(Throwable ignored) {
			return;
		}

		new Hackontrol(bot);
	}
}
