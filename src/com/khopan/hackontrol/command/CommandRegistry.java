package com.khopan.hackontrol.command;

public interface CommandRegistry {
	void register(Class<? extends Command> commandClass);
}
