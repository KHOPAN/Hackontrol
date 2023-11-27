package com.khopan.hackontrol.command;

import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;

public interface Command {
	void register(CommandDispatcher<CommandSource> dispatcher);
}
