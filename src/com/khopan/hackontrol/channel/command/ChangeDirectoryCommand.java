package com.khopan.hackontrol.channel.command;

import java.io.File;

import com.khopan.hackontrol.manager.common.sender.sendable.ISendable;

import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class ChangeDirectoryCommand {
	private ChangeDirectoryCommand() {}

	public static boolean test(String command) {
		return (command.length() == 2 && command.startsWith("cd")) || command.startsWith("cd ");
	}

	public static void execute(ISendable sender, String command) {
		if(command.length() == 2) {
			String directory = CommandChannel.CurrentDirectory == null ? "SYSTEMROOT" : CommandChannel.CurrentDirectory.getAbsolutePath();
			sender.send(MessageCreateData.fromContent('`' + directory + '`'), null);
			return;
		}

		command = command.substring(3).trim();

		if(command.isEmpty()) {
			sender.send(MessageCreateData.fromContent("`Command input is empty`"), null);
			return;
		}

		File file = new File(command);

		if(!file.exists()) {
			sender.send(MessageCreateData.fromContent("`The system cannot find the path specified.`"), null);
			return;
		}

		if(!file.isDirectory()) {
			sender.send(MessageCreateData.fromContent("`The directory name is invalid.`"), null);
			return;
		}

		CommandChannel.CurrentDirectory = file;
		sender.send(MessageCreateData.fromContent('`' + file.getAbsolutePath() + '`'), null);
	}
}
