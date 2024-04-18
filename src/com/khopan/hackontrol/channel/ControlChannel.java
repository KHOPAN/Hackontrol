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
		registry.register(ControlChannel.SLEEP_BUTTON_IDENTIFIER, interaction -> this.power(interaction, PowerAction.SLEEP));
		registry.register(ControlChannel.SHUTDOWN_BUTTON_IDENTIFIER, interaction -> this.power(interaction, PowerAction.SHUTDOWN));
		registry.register(ControlChannel.RESTART_BUTTON_IDENTIFIER, interaction -> this.power(interaction, PowerAction.RESTART));
	}

	private void power(ButtonInteraction interaction, PowerAction powerAction) {
		interaction.yesNoQuestion("Are you sure you want to " + powerAction.text + '?', answer -> this.powerCallback(interaction, powerAction, answer));
	}

	private void powerCallback(ButtonInteraction interaction, PowerAction powerAction, boolean answer) {
		System.out.println(powerAction.text);
	}

	private static enum PowerAction {
		SLEEP("sleep"),
		SHUTDOWN("shutdown"),
		RESTART("restart");

		private final String text;

		PowerAction(String text) {
			this.text = text;
		}
	}
}
