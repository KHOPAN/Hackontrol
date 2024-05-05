package com.khopan.hackontrol.channel;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.button.ButtonContext;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.registry.Registry;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;

public class CameraChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "camera";

	private static final Button CAMERA_LIST_BUTTON = ButtonManager.staticButton(ButtonStyle.SUCCESS, "Camera List", "cameraList");

	@Override
	public String getName() {
		return CameraChannel.CHANNEL_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, CameraChannel.CAMERA_LIST_BUTTON, this :: buttonCameraList);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(CameraChannel.CAMERA_LIST_BUTTON)).queue();
	}

	private void buttonCameraList(ButtonContext context) {
		context.reply("**Not Implemented**").queue();
	}
}
