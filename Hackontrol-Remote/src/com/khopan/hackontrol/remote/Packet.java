package com.khopan.hackontrol.remote;

import java.io.IOException;
import java.io.InputStream;

public class Packet {
	public static final int PACKET_TYPE_SCREEN_INFORMATION = 1;
	public static final int PACKET_TYPE_STREAM_FRAME       = 2;

	private final int size;
	private final int type;
	private final byte[] data;

	private Packet(int size, int type, byte[] data) {
		this.size = size;
		this.type = type;
		this.data = data;
	}

	public int getSize() {
		return this.size;
	}

	public int getType() {
		return this.type;
	}

	public byte[] getData() {
		return this.data;
	}

	public static Packet readPacket(InputStream stream) throws IOException {
		byte[] bytes = stream.readNBytes(5);
		int type = bytes[4] & 0xFF;

		if(type < Packet.PACKET_TYPE_SCREEN_INFORMATION || type > Packet.PACKET_TYPE_STREAM_FRAME) {
			throw new IllegalArgumentException("Unknown packet type: " + type);
		}

		int size = ((bytes[0] & 0xFF) << 24) | ((bytes[1] & 0xFF) << 16) | ((bytes[2] & 0xFF) << 8) | (bytes[3] & 0xFF);
		return new Packet(size, type, stream.readNBytes(size));
	}
}
