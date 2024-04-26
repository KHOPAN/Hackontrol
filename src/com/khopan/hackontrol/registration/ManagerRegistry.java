package com.khopan.hackontrol.registration;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.manager.modal.ModalManager;
import com.khopan.hackontrol.registry.Registry;

public class ManagerRegistry {
	private ManagerRegistry() {}

	public static void register(Registry registry) {
		registry.register(Hackontrol.MANAGER_REGISTRY, ButtonManager.class);
		registry.register(Hackontrol.MANAGER_REGISTRY, ModalManager.class);
	}
}
