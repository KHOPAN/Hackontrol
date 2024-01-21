package com.khopan.hackontrol.command;

import java.awt.Desktop;
import java.net.URI;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;

public class RickrollCommand implements Command {
	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("rickroll").executes(context -> {
			CommandSource source = (CommandSource) context.getSource();

			try {
				Desktop.getDesktop().browse(new URI("https://www.youtube.com/watch?v=dQw4w9WgXcQ"));
				source.sendMessage("Successfully Rickrolled!");
			} catch(Throwable ignored) {
				source.sendMessage("Rickroll has failed!");
				return -1;
			}

			return 1;
		}));
	}
}
