package com.khopan.hackontrol.channel;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class HackontrolDiscordChannel extends HackontrolChannel {
	private static String CHANNEL_NAME = "hackontrol";

	private static final Button PING_BUTTON = ButtonManager.staticButton(ButtonType.SUCCESS, "Ping", "ping");

	@Override
	public String getName() {
		return HackontrolDiscordChannel.CHANNEL_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, HackontrolDiscordChannel.PING_BUTTON, context -> HackontrolMessage.deletable(context.reply(), "**Status: Ok**"));
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(HackontrolDiscordChannel.PING_BUTTON)).queue();
	}

	@Override
	public void postInitialize() {
		long time = System.currentTimeMillis() / 1000L;
		this.channel.sendMessage("**Online: <t:" + time + ":f> (<t:" + time + ":R>)**").queue();
	}
}
