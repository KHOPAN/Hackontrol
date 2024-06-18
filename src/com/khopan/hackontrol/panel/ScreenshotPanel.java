package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.widget.ControlWidget;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class ScreenshotPanel extends Panel {
	private static final String PANEL_NAME = "control";

	private static final Button BUTTON_SCREENSHOT = ButtonManager.staticButton(ButtonType.SUCCESS, "Screenshot", "screenshot");
	private static final Button BUTTON_REFRESH    = ButtonManager.staticButton(ButtonType.SUCCESS, "Refresh",    "screenshotRefresh");

	@Override
	public String panelName() {
		return ScreenshotPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, ScreenshotPanel.BUTTON_SCREENSHOT, context -> {});
		this.register(Registration.BUTTON, ScreenshotPanel.BUTTON_REFRESH,    context -> {});
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(ScreenshotPanel.BUTTON_SCREENSHOT)
				.build()
		};
	}
}
