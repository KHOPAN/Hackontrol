package com.khopan.hackontrol.button;

import java.util.function.Consumer;

@FunctionalInterface
public interface ButtonHandlerRegistry {
	void register(String buttonIdentifier, Consumer<ButtonInteraction> action);
}
