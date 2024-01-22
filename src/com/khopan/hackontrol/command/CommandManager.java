package com.khopan.hackontrol.command;

import com.khopan.hackontrol.command.popcorn.ScreenshotCommand;

public class CommandManager {
	public static void register(CommandRegistry registry) {
		registry.register(ScreenshotCommand.class);
	}
}
