package com.khopan.hackontrol.eventlistener;

import java.util.function.Consumer;

import net.dv8tion.jda.api.events.GenericEvent;
import net.dv8tion.jda.api.hooks.EventListener;

public class FilteredEventListener implements EventListener {
	private final Class<? extends GenericEvent> eventClass;
	private final Consumer<Object> action;

	@SuppressWarnings("unchecked")
	private FilteredEventListener(Class<? extends GenericEvent> eventClass, Consumer<? extends GenericEvent> action) {
		this.eventClass = eventClass;
		this.action = (Consumer<Object>) action;
	}

	@Override
	public void onEvent(GenericEvent Event) {
		if(this.eventClass.isAssignableFrom(Event.getClass())) {
			this.action.accept(Event);
		}
	}

	public static <T extends GenericEvent> FilteredEventListener create(Class<T> eventClass, Consumer<T> action) {
		if(eventClass == null || action == null) {
			return null;
		}

		return new FilteredEventListener(eventClass, action);
	}
}
