package com.khopan.hackontrol.command;

import com.khopan.hackontrol.nativelib.power.Power;
import com.khopan.hackontrol.nativelib.power.ShutdownReason;
import com.khopan.hackontrol.nativelib.power.ShutdownType;
import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;

public class PowerCommand implements Command {
	private static final long SHUTDOWN_REASON = ShutdownReason.SHTDN_REASON_MAJOR_SYSTEM | ShutdownReason.SHTDN_REASON_MINOR_PROCESSOR;

	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("power").requires(source -> source.isSelected()).then(LiteralArgumentBuilder.<CommandSource>literal("hybrid").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_HYBRID_SHUTDOWN | ShutdownType.EWX_SHUTDOWN, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("logoff").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_LOGOFF, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("poweroff").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_POWEROFF, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("reboot").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_REBOOT, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("restartapps").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_RESTARTAPPS, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("shutdown").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_SHUTDOWN, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("force").then(LiteralArgumentBuilder.<CommandSource>literal("hybrid").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_FORCE | ShutdownType.EWX_HYBRID_SHUTDOWN | ShutdownType.EWX_SHUTDOWN, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("logoff").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_FORCE | ShutdownType.EWX_LOGOFF, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("poweroff").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_FORCE | ShutdownType.EWX_POWEROFF, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("reboot").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_FORCE | ShutdownType.EWX_REBOOT, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("restartapps").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_FORCE | ShutdownType.EWX_RESTARTAPPS, PowerCommand.SHUTDOWN_REASON);
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("shutdown").executes(context -> {
			Power.ExitWindowsEx(ShutdownType.EWX_FORCE | ShutdownType.EWX_SHUTDOWN, PowerCommand.SHUTDOWN_REASON);
			return 1;
		}))));
	}
}
