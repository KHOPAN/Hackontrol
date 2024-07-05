package com.khopan.hackontrol.utils.event;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.utils.ErrorHandler;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.events.GenericEvent;
import net.dv8tion.jda.api.hooks.EventListener;
import net.dv8tion.jda.api.interactions.Interaction;

public class InteractionEventListener<T extends GenericEvent & Interaction> implements EventListener {
	private final Class<T> eventClass;
	private final ThrowableAction<T> action;

	private InteractionEventListener(Class<T> eventClass, ThrowableAction<T> action) {
		this.eventClass = eventClass;
		this.action = action;
	}

	@SuppressWarnings("unchecked")
	@Override
	public void onEvent(GenericEvent Event) {
		if(this.action == null) {
			return;
		}

		try {
			if(this.eventClass == null) {
				this.action.onEvent((T) Event);
			}

			if(!(Event instanceof Interaction interaction && HackontrolMessage.checkCategory(interaction.getChannel()))) {
				return;
			}

			if(this.eventClass.isAssignableFrom(Event.getClass())) {
				this.action.onEvent((T) Event);
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

	public static <T extends GenericEvent & Interaction> EventListener create(Class<T> eventClass, ThrowableAction<T> action) {
		return new InteractionEventListener<T>(eventClass, action);
	}
}
