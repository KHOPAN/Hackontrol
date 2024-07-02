package com.khopan.hackontrol.registration;

import com.khopan.hackontrol.service.MessageService;
import com.khopan.hackontrol.service.ServiceManager;
import com.khopan.hackontrol.service.interaction.ButtonService;
import com.khopan.hackontrol.service.interaction.ModalService;

public class ServiceRegistry {
	private ServiceRegistry() {}

	public static void register(ServiceManager manager) {
		manager.register(new ButtonService());
		manager.register(new ModalService());
		manager.register(new MessageService());
	}
}
