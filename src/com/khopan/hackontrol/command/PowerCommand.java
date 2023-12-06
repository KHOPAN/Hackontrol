package com.khopan.hackontrol.command;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;

public class PowerCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("power").requires(source -> source.isSelected()).then(LiteralArgumentBuilder.<CommandSource>literal("hybrid").executes(context -> {
			context.getSource().message("Hybrid shutdown");
			PowerCommand.exitWindows(0x400001);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("logoff").executes(context -> {
			context.getSource().message("Log off");
			PowerCommand.exitWindows(0x000000);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("poweroff").executes(context -> {
			context.getSource().message("Power off");
			PowerCommand.exitWindows(0x000008);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("reboot").executes(context -> {
			context.getSource().message("Reboot");
			PowerCommand.exitWindows(0x000002);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("restartapps").executes(context -> {
			context.getSource().message("Restart apps");
			PowerCommand.exitWindows(0x000040);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("shutdown").executes(context -> {
			context.getSource().message("Shutdown");
			PowerCommand.exitWindows(0x000001);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("force").then(LiteralArgumentBuilder.<CommandSource>literal("hybrid").executes(context -> {
			context.getSource().message("Force hybrid shutdown");
			PowerCommand.exitWindows(0x400005);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("logoff").executes(context -> {
			context.getSource().message("Force log off");
			PowerCommand.exitWindows(0x000004);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("poweroff").executes(context -> {
			context.getSource().message("Force power off");
			PowerCommand.exitWindows(0x00000C);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("reboot").executes(context -> {
			context.getSource().message("Force reboot");
			PowerCommand.exitWindows(0x000006);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("restartapps").executes(context -> {
			context.getSource().message("Force restart apps");
			PowerCommand.exitWindows(0x000044);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("shutdown").executes(context -> {
			context.getSource().message("Force shutdown");
			PowerCommand.exitWindows(0x000005);
			return 1;
		}))));
	}

	private static native void exitWindows(int flags);
}
