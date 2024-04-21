package com.khopan.hackontrol;

import com.khopan.hackontrol.registry.Registry;

import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;

public abstract class HackontrolChannel {
	protected TextChannel channel;

	public abstract String getName();

	public void initialize() {

	}

	public void register(Registry registry) {

	}
}
