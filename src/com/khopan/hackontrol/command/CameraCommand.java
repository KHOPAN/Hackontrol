package com.khopan.hackontrol.command;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.arguments.IntegerArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;

public class CameraCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("camera").requires(source -> source.isSelected()).executes(context -> {
			return 1;
		}).then(LiteralArgumentBuilder.<CommandSource>literal("list").executes(context -> {
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("select").then(RequiredArgumentBuilder.<CommandSource, Integer>argument("index", IntegerArgumentType.integer(0)).executes(context -> {
			return 1;
		}))));
	}
}
