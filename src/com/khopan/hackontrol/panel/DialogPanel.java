package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class DialogPanel extends Panel {
	private static final String PANEL_NAME = "dialog";

	private static Button BUTTON_NEW_DIALOG = ButtonManager.staticButton(ButtonType.SUCCESS, "New Dialog", "newDialog");

	@Override
	public String panelName() {
		return DialogPanel.PANEL_NAME;
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(DialogPanel.BUTTON_NEW_DIALOG)
				.build()
		};
	}
}
