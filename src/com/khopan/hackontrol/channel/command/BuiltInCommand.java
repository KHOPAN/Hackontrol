package com.khopan.hackontrol.channel.command;

import com.khopan.hackontrol.manager.common.sender.sendable.ISendable;

public class BuiltInCommand {
	private BuiltInCommand() {}

	public static boolean execute(ISendable sender, String command) {
		if(ChangeDirectoryCommand.test(command)) {
			ChangeDirectoryCommand.execute(sender, command);
			return true;
		}

		return false;
	}
}
