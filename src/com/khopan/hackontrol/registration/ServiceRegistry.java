package com.khopan.hackontrol.registration;

import com.khopan.hackontrol.service.ServiceManager;
import com.khopan.hackontrol.service.interaction.ButtonService;
import com.khopan.hackontrol.service.interaction.MessageService;
import com.khopan.hackontrol.service.interaction.ModalService;
import com.khopan.hackontrol.service.interaction.StringSelectMenuService;

public class ServiceRegistry {
	private ServiceRegistry() {}

	public static void register(ServiceManager manager) {
		manager.register(new MessageService());
		manager.register(new ButtonService());
		manager.register(new ModalService());
		manager.register(new StringSelectMenuService());
	}
}
