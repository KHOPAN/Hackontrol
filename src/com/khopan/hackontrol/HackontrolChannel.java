package com.khopan.hackontrol;

import com.khopan.hackontrol.registry.Registry;

import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;

public abstract class HackontrolChannel {
	protected Hackontrol hackontrol;
	protected Category category;
	protected TextChannel channel;

	public abstract String getName();

	public void preInitialize(Registry registry) {

	}

	public void initialize() {

	}

	public void postInitialize() {

	}
}
