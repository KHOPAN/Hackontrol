package com.khopan.hackontrol;

import com.khopan.hackontrol.button.ButtonHandlerRegistry;

import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;

public abstract class HackontrolChannel {
	protected TextChannel channel;

	public abstract String getChannelName();
	public abstract void sendInitializeMessage();
	public abstract void registerButtonHandler(ButtonHandlerRegistry registry);
}
