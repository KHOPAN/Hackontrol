package com.khopan.hackontrol.command;

import java.util.Iterator;
import java.util.Map;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.arguments.StringArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;
import com.mojang.brigadier.tree.CommandNode;

public class HelpCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("help").executes(context -> {
			CommandSource source = context.getSource();
			Map<CommandNode<CommandSource>, String> smartUsage = dispatcher.getSmartUsage(dispatcher.getRoot(), source);
			Iterator<String> usageIterator = smartUsage.values().iterator();
			StringBuilder builder = new StringBuilder();
			builder.append("Commands:");

			while(usageIterator.hasNext()) {
				String usage = usageIterator.next();
				builder.append("\n$");
				builder.append(usage);
			}

			String usageList = builder.toString();
			source.sendCodeMessage(usageList);
			return 1;
		}).then(RequiredArgumentBuilder.<CommandSource, String>argument("command", StringArgumentType.greedyString()).executes(context -> {
			return 1;
		})));
	}
}
