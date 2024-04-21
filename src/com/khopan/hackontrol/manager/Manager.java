package com.khopan.hackontrol.manager;

import com.khopan.hackontrol.registry.RegistrationHandler;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;

public interface Manager {
	default void configureBuilder(JDABuilder builder) {

	}

	default void configureBot(JDA bot) {

	}

	default void initialize(RegistrationHandler handler) {

	}
}
