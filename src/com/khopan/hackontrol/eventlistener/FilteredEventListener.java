package com.khopan.hackontrol.eventlistener;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.utils.ErrorHandler;

import net.dv8tion.jda.api.events.GenericEvent;
import net.dv8tion.jda.api.hooks.EventListener;

public class FilteredEventListener implements EventListener {
	private final Class<? extends GenericEvent> eventClass;
	private final ThrowableAction<GenericEvent> action;

	@SuppressWarnings("unchecked")
	private FilteredEventListener(Class<? extends GenericEvent> eventClass, ThrowableAction<? extends GenericEvent> action) {
		this.eventClass = eventClass;
		this.action = (ThrowableAction<GenericEvent>) action;
	}

	@Override
	public void onEvent(GenericEvent Event) {
		if(this.action == null) {
			return;
		}

		try {
			if(this.eventClass == null || this.eventClass.isAssignableFrom(Event.getClass())) {
				this.action.onEvent(Event);
			}
		} catch(Throwable Errors) {
			Hackontrol hackontrol = Hackontrol.getInstance();
			ErrorHandler handler = hackontrol.getErrorHandler();

			if(handler != null) {
				handler.errorOccured(Thread.currentThread(), Errors);
			} else {
				throw new RuntimeException("Event listener error", Errors);
			}
		}
	}

	public static <T extends GenericEvent> EventListener create(Class<T> eventClass, ThrowableAction<T> action) {
		return new FilteredEventListener(eventClass, action);
	}
}
