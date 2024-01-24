package com.khopan.hackontrol.command.bullet;

import com.khopan.hackontrol.command.BulletCommand;

import net.dv8tion.jda.api.events.interaction.command.SlashCommandInteractionEvent;
import net.dv8tion.jda.api.interactions.commands.build.Commands;
import net.dv8tion.jda.api.interactions.commands.build.SlashCommandData;

public class BluescreenCommand implements BulletCommand {
	@Override
	public SlashCommandData getCommand() {
		return Commands.slash("bluescreen", "Cause a bluescreen of death to appear on the target computer")
				.setGuildOnly(false);
	}

	@Override
	public void handleCommand(SlashCommandInteractionEvent Event) {

	}
}
