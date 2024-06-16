package com.khopan.hackontrol.service;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.khopan.hackontrol.panel.PanelManager;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;

public class ServiceManager {
	private final List<Service> serviceList;

	public ServiceManager() {
		this.serviceList = new ArrayList<>();
	}

	public void register(Service service) {
		if(service == null) {
			throw new NullPointerException("Service cannot be null");
		}

		this.serviceList.add(service);
	}

	public List<Service> serviceList() {
		return Collections.unmodifiableList(this.serviceList);
	}

	public void applyBuilder(JDABuilder builder, PanelManager manager) {
		if(builder == null) {
			throw new NullPointerException("Builder cannot be null");
		}

		if(manager == null) {
			throw new NullPointerException("Manager cannot be null");
		}

		for(Service service : this.serviceList) {
			service.panelManager = manager;
		}

		for(Service service : this.serviceList) {
			service.preBuild(builder);
		}
	}

	public void initialize(JDA bot) {
		if(bot == null) {
			throw new NullPointerException("Bot cannot be null");
		}

		for(Service service : this.serviceList) {
			service.postBuild(bot);
		}

		for(Service service : this.serviceList) {
			service.initialize();
		}
	}
}
