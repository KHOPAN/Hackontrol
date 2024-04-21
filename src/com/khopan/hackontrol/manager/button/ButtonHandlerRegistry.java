package com.khopan.hackontrol.manager.button;

import java.util.function.Consumer;

@FunctionalInterface
public interface ButtonHandlerRegistry {
	void register(String buttonIdentifier, Consumer<ButtonInteraction> action);
}
