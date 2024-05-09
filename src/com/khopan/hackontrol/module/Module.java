package com.khopan.hackontrol.module;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.registry.Registry;

import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;

public abstract class Module {
	public Hackontrol hackontrol;
	public Category category;
	public TextChannel channel;

	public abstract String getName();

	public void preInitialize(Registry registry) {

	}

	public void initialize() {

	}

	public void postInitialize() {

	}
}
