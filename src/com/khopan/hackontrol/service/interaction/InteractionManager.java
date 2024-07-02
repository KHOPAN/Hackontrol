package com.khopan.hackontrol.service.interaction;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.interactions.InteractionHook;

public class InteractionManager {
	private InteractionManager() {}

	public static void callback(Message message) {
		ButtonService.assignIdentifier(message);
		StringSelectMenuService.assignIdentifier(message);
	}

	public static void callback(InteractionHook hook) {
		hook.retrieveOriginal().queue(InteractionManager :: callback);
	}
}
