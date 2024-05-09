package com.khopan.hackontrol.module.command;

import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;

public class BuiltInCommand {
	private BuiltInCommand() {}

	public static boolean execute(TextChannel channel, String command) {
		if(ChangeDirectoryCommand.test(command)) {
			ChangeDirectoryCommand.execute(channel, command);
			return true;
		} else if(ClearCommand.test(command)) {
			ClearCommand.execute(channel);
			return true;
		}

		return false;
	}
}
