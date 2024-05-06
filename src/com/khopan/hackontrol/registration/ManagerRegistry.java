package com.khopan.hackontrol.registration;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.manager.MessageManager;
import com.khopan.hackontrol.manager.command.CommandManager;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.registry.Registry;

public class ManagerRegistry {
	private ManagerRegistry() {}

	public static void register(Registry registry) {
		registry.register(Hackontrol.MANAGER_REGISTRY, InteractionManager.class);
		registry.register(Hackontrol.MANAGER_REGISTRY, CommandManager.class);
		registry.register(Hackontrol.MANAGER_REGISTRY, MessageManager.class);
	}
}
