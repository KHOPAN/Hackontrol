package com.khopan.hackontrol.module;

import com.khopan.hackontrol.NativeLibrary;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.manager.interaction.ModalContext;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolError;

import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.interactions.modals.Modal;
import net.dv8tion.jda.api.interactions.modals.ModalMapping;

public class AudioModule extends Module {
	private static final String MODULE_NAME = "audio";

	private static final Button BUTTON_CHANGE_VOLUME = ButtonManager.staticButton(ButtonType.SUCCESS, "Change Volume", "changeVolume");

	private static final String MODAL_CHANGE_VOLUME = "changeVolume";

	@Override
	public String getName() {
		return AudioModule.MODULE_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, AudioModule.BUTTON_CHANGE_VOLUME, this :: buttonChangeVolume);
		registry.register(InteractionManager.MODAL_REGISTRY,  AudioModule.MODAL_CHANGE_VOLUME,  this :: modalChangeVolume);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(AudioModule.BUTTON_CHANGE_VOLUME)).queue();
	}

	private void buttonChangeVolume(ButtonContext context) {
		String volumeLevel = Integer.toString(Math.min(Math.max((int) Math.round(((double) NativeLibrary.currentVolume()) * 100.0d), 0), 100));
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
		NativeLibrary.volume(volume);
		context.deferEdit().queue();
	}
}
