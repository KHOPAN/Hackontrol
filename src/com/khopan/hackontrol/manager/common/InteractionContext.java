package com.khopan.hackontrol.manager.common;

import java.util.function.Consumer;

import net.dv8tion.jda.api.interactions.InteractionHook;

public interface InteractionContext {
	void delete();
	void acknowledge();
	void acknowledge(Consumer<? super InteractionHook> callback);
	void thinking();
	void thinking(Consumer<? super InteractionHook> callback);
}
