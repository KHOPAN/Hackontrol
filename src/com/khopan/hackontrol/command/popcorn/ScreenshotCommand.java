package com.khopan.hackontrol.command.popcorn;

import com.khopan.hackontrol.command.PopcornCommand;

import net.dv8tion.jda.api.entities.User;
import net.dv8tion.jda.api.events.interaction.command.SlashCommandInteractionEvent;
import net.dv8tion.jda.api.interactions.commands.build.Commands;
import net.dv8tion.jda.api.interactions.commands.build.SlashCommandData;

public class ScreenshotCommand implements PopcornCommand {
	@Override
	public SlashCommandData getCommand() {
		return Commands.slash("screenshot", "Screenshot the target computer")
				.setGuildOnly(false);
	}

	@Override
	public void handleCommand(SlashCommandInteractionEvent Event) {
		User user = Event.getUser();
		System.out.println(user.getGlobalName() + " used /screenshot");
		Event.reply("Hello, world!\nThere is no screenshot!").queue();
	}
}
