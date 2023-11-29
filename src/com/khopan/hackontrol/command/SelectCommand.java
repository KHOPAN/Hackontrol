package com.khopan.hackontrol.command;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.arguments.StringArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;

public class SelectCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("select").then(RequiredArgumentBuilder.<CommandSource, String>argument("machineId", StringArgumentType.word()).executes(context -> {
			CommandSource source = context.getSource();
			String machineIdArgument = StringArgumentType.getString(context, "machineId");
			String machineId = source.getMachineId();

			if(machineId.equals(machineIdArgument)) {
				source.setSelected(true);
			}

			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("all").executes(context -> {
			CommandSource source = context.getSource();
			source.setSelected(true);
			return 1;
		})));
	}
}
