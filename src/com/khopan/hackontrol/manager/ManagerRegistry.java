package com.khopan.hackontrol.manager;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.registry.Registry;

public class ManagerRegistry {
	private ManagerRegistry() {}

	public static void register(Registry registry) {
		registry.register(Hackontrol.MANAGER_REGISTRY, ButtonManager.class);
	}
}
