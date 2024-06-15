package com.khopan.hackontrol.service;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;

public interface Service {
	default void preBuild(JDABuilder builder) {

	}

	default void postBuild(JDA bot) {

	}

	default void initialize() {

	}
}
