package com.khopan.hackontrol.command;

import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.LiteralMessage;
import com.mojang.brigadier.ParseResults;
import com.mojang.brigadier.arguments.StringArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;
import com.mojang.brigadier.context.ParsedCommandNode;
import com.mojang.brigadier.exceptions.SimpleCommandExceptionType;
import com.mojang.brigadier.tree.CommandNode;

public class HelpCommand implements Command {
	private static final SimpleCommandExceptionType ERROR_INVALID_COMMAND = new SimpleCommandExceptionType(new LiteralMessage("Invalid command"));

	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("help").executes(context -> {
			CommandSource source = context.getSource();
			source.sendCodeMessage(this.smartUsage(dispatcher, dispatcher.getRoot(), source, false, "EMPTY", null));
			return 1;
		}).then(RequiredArgumentBuilder.<CommandSource, String>argument("command", StringArgumentType.greedyString()).executes(context -> {
			CommandSource source = context.getSource();
			String command = StringArgumentType.getString(context, "command");
			ParseResults<CommandSource> parseResults = dispatcher.parse(command, source);
			List<ParsedCommandNode<CommandSource>> list = parseResults.getContext().getNodes();

			if(list.isEmpty()) {
				throw HelpCommand.ERROR_INVALID_COMMAND.create();
			}

			ParsedCommandNode<CommandSource> last = list.get(list.size() - 1);
			CommandNode<CommandSource> node = last.getNode();
			String prefix = parseResults.getReader().getString();
			source.sendCodeMessage(this.smartUsage(dispatcher, node, source, true, command, prefix));
			return 1;
		})));
	}

	private String smartUsage(CommandDispatcher<CommandSource> dispatcher, CommandNode<CommandSource> node, CommandSource source, boolean command, String thatCommand, String prefix) {
		Map<CommandNode<CommandSource>, String> smartUsage = dispatcher.getSmartUsage(node, source);
		Iterator<String> usageIterator = smartUsage.values().iterator();
		StringBuilder builder = new StringBuilder();

		if(command) {
			builder.append("Usage:");
		} else {
			builder.append("Commands:");
		}

		int count = 0;

		while(usageIterator.hasNext()) {
			String usage = usageIterator.next();
			builder.append("\n$");

			if(prefix != null) {
				builder.append(prefix);
				builder.append(' ');
			}

			builder.append(usage);
			count++;
		}

		if(count < 1) {
			builder.append("\n$");
			builder.append(thatCommand);
		}

		return builder.toString();
	}
}
