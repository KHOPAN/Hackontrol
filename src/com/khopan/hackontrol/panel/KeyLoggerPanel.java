package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class KeyLoggerPanel extends Panel {
	private static final String PANEL_NAME = "keylogger";

	private static final Button BUTTON_ENABLE  = ButtonManager.staticButton(ButtonType.SUCCESS, "Enable",  "keyloggerEnable");
	private static final Button BUTTON_DISABLE = ButtonManager.staticButton(ButtonType.DANGER,  "Disable", "keyloggerDisable");

	@Override
	public String panelName() {
		return KeyLoggerPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {

	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(KeyLoggerPanel.BUTTON_ENABLE, KeyLoggerPanel.BUTTON_DISABLE)
				.build()
		};
	}
}
