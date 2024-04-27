package com.khopan.hackontrol.manager.common;

import java.util.function.Consumer;

import net.dv8tion.jda.api.interactions.InteractionHook;

public interface IThinkable {
	void thinking();
	void thinking(Consumer<? super InteractionHook> callback);
}
