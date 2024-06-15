package com.khopan.hackontrol.registration;

import com.khopan.hackontrol.service.ServiceManager;
import com.khopan.hackontrol.service.interaction.ButtonService;

public class ServiceRegistry {
	private ServiceRegistry() {}

	public static void register(ServiceManager manager) {
		manager.register(new ButtonService());
	}
}
