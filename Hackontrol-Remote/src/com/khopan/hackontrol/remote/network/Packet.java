package com.khopan.hackontrol.remote.network;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class Packet {
	public static final int PACKET_TYPE_INFORMATION  = 1;
	public static final int PACKET_TYPE_STREAM_FRAME = 2;

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
		if(stream == null) {
			throw new NullPointerException("Stream cannot be null");
		}

		byte[] bytes;

		do {
			bytes = stream.readNBytes(5);
		} while(bytes.length < 5);

		int type = bytes[4] & 0xFF;
		Packet.checkPacketType(type);
		int size = ((bytes[0] & 0xFF) << 24) | ((bytes[1] & 0xFF) << 16) | ((bytes[2] & 0xFF) << 8) | (bytes[3] & 0xFF);

		if(size < 1) {
			return new Packet(0, type, null);
		}

		return new Packet(size, type, stream.readNBytes(size));
	}

	public static void writePacket(OutputStream stream, Packet packet) throws IOException {
		if(stream == null) {
			throw new NullPointerException("Stream cannot be null");
		}

		if(packet == null) {
			throw new NullPointerException("Packet cannot be null");
		}

		stream.write((packet.size >> 24) & 0xFF);
		stream.write((packet.size >> 16) & 0xFF);
		stream.write((packet.size >> 8) & 0xFF);
		stream.write(packet.size & 0xFF);
		stream.write(packet.type & 0xFF);

		if(packet.size > 0) {
			stream.write(packet.data);
		}
	}

	public static Packet of(byte[] data, int type) {
		Packet.checkPacketType(type);
		return new Packet(data == null ? 0 : data.length, type, data);
	}

	private static void checkPacketType(int type) {
		if(type < Packet.PACKET_TYPE_INFORMATION || type > Packet.PACKET_TYPE_STREAM_FRAME) {
			throw new IllegalArgumentException("Unknown packet type: " + type);
		}
	}
}
