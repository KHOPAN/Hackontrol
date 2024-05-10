package com.khopan.hackontrol.persistent.file;

import java.io.ByteArrayOutputStream;

import com.khopan.hackontrol.persistent.IPersistentWriter;

class FilePersistentWriter implements IPersistentWriter {
	private final ByteArrayOutputStream stream;

	FilePersistentWriter() {
		this.stream = new ByteArrayOutputStream();
	}

	@Override
	public void writeBoolean(boolean value) {
		this.writeByte((byte) (value ? 1 : 0));
	}

	@Override
	public void writeByte(byte value) {
		this.stream.write(value);
	}

	@Override
	public void writeChar(char value) {
		this.writeShort((short) value);
	}

	@Override
	public void writeShort(short value) {
		this.writeByte((byte) ((value >> 8) & 0xFF));
		this.writeByte((byte) (value & 0xFF));
	}

	@Override
	public void writeFloat(float value) {
		this.writeInt(Float.floatToIntBits(value));
	}

	@Override
	public void writeInt(int value) {
		this.writeByte((byte) ((value >> 24) & 0xFF));
		this.writeByte((byte) ((value >> 16) & 0xFF));
		this.writeByte((byte) ((value >> 8) & 0xFF));
		this.writeByte((byte) (value & 0xFF));
	}

	@Override
	public void writeDouble(double value) {
		this.writeLong(Double.doubleToLongBits(value));
	}

	@Override
	public void writeLong(long value) {
		this.writeByte((byte) ((value >> 56) & 0xFF));
		this.writeByte((byte) ((value >> 48) & 0xFF));
		this.writeByte((byte) ((value >> 40) & 0xFF));
		this.writeByte((byte) ((value >> 32) & 0xFF));
		this.writeByte((byte) ((value >> 24) & 0xFF));
		this.writeByte((byte) ((value >> 16) & 0xFF));
		this.writeByte((byte) ((value >> 8) & 0xFF));
		this.writeByte((byte) (value & 0xFF));
	}

	void writeWriter(FilePersistentWriter writer) {
		byte[] data = writer.getData();
		this.writeByteArray(data);
	}

	byte[] getData() {
		return this.stream.toByteArray();
	}
}
