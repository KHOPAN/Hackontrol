package com.khopan.hackontrol.command;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.arguments.StringArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;

public class WakeCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("wake").requires(source -> source.isSelected()).then(RequiredArgumentBuilder.<CommandSource, String>argument("ip", StringArgumentType.greedyString()).executes(context -> {
			CommandSource source = context.getSource();
			String ip = StringArgumentType.getString(context, "ip");
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("all").executes(context -> {
			return 1;
		})));
	}

	private static native void wakeAddress(CommandSource source, int ipAddress);
	private static native void wakeAll(CommandSource source);
}
