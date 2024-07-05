package com.khopan.hackontrol.utils.event;

import net.dv8tion.jda.api.events.GenericEvent;

@FunctionalInterface
public interface ThrowableAction<T extends GenericEvent> {
	void onEvent(T Event) throws Throwable;
}
