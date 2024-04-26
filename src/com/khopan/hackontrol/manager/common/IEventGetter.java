package com.khopan.hackontrol.manager.common;

import net.dv8tion.jda.api.events.Event;

public interface IEventGetter<T extends Event> {
	T getEvent();
}
