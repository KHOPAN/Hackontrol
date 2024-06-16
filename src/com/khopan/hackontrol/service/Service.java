package com.khopan.hackontrol.service;

import com.khopan.hackontrol.panel.PanelManager;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;

public abstract class Service {
	protected PanelManager panelManager;

	public void preBuild(JDABuilder builder) {

	}

	public void postBuild(JDA bot) {

	}

	public void initialize() {

	}
}
