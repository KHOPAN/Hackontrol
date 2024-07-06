package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class ImagePanel extends Panel {
	private static final String PANEL_NAME = "image";

	private static final Button BUTTON_UPLOAD = ButtonManager.staticButton(ButtonType.SUCCESS, "Upload", "imageUpload");

	@Override
	public String panelName() {
		return ImagePanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, ImagePanel.BUTTON_UPLOAD, context -> context.deferEdit().queue());
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(ImagePanel.BUTTON_UPLOAD)
				.build()
		};
	}
}
