package com.khopan.hackontrol.channel;

import com.khopan.hackontrol.ButtonHandlerRegistry;
import com.khopan.hackontrol.ButtonInteraction;
import com.khopan.hackontrol.HackontrolChannel;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class ControlChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "control";

	private static final String SLEEP_BUTTON_IDENTIFIER = "controlSleep";
	private static final String SHUTDOWN_BUTTON_IDENTIFIER = "controlShutdown";
	private static final String RESTART_BUTTON_IDENTIFIER = "controlRestart";

	public ControlChannel() {

	}

	@Override
	public String getChannelName() {
		return ControlChannel.CHANNEL_NAME;
	}

	@Override
	public void sendInitializeMessage() {
		this.channel.sendMessage("**Power Control**").addActionRow(Button.success(ControlChannel.SLEEP_BUTTON_IDENTIFIER, "Sleep"), Button.danger(ControlChannel.SHUTDOWN_BUTTON_IDENTIFIER, "Shutdown"), Button.primary(ControlChannel.RESTART_BUTTON_IDENTIFIER, "Restart")).queue();
	}

	@Override
	public void registerButtonHandler(ButtonHandlerRegistry registry) {
		registry.register(ControlChannel.SLEEP_BUTTON_IDENTIFIER, this :: sleep);
		registry.register(ControlChannel.SHUTDOWN_BUTTON_IDENTIFIER, this :: shutdown);
		registry.register(ControlChannel.RESTART_BUTTON_IDENTIFIER, this :: restart);
	}

	private void sleep(ButtonInteraction interaction) {
		interaction.consume();
		System.out.println("Sleep");
	}

	private void shutdown(ButtonInteraction interaction) {
		interaction.consume();
		System.out.println("Shutdown");
	}

	private void restart(ButtonInteraction interaction) {
		interaction.consume();
		System.out.println("Restart");
	}
}
