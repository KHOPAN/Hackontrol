package com.khopan.hackontrol.persistent.file;

import com.khopan.hackontrol.persistent.IPersistentReader;

class FilePersistentReader implements IPersistentReader {
	private final byte[] data;
	private final int length;

	private int pointer;

	FilePersistentReader(byte[] data) {
		this.data = data;
		this.length = this.data.length;
		this.pointer = 0;
	}

	@Override
	public boolean readBoolean() {
		return this.readByte() != 0;
	}

	@Override
	public byte readByte() {
		if(this.pointer >= this.length) {
			throw new IndexOutOfBoundsException("No more data");
		}

		byte value = this.data[this.pointer];
		this.pointer++;
		return value;
	}

	@Override
	public char readChar() {
		return (char) this.readShort();
	}

	@Override
	public short readShort() {
		return (short) (((this.readByte() & 0xFF) << 8) | (this.readByte() & 0xFF));
	}

	@Override
	public float readFloat() {
		return Float.intBitsToFloat(this.readInt());
	}

	@Override
	public int readInt() {
		return ((this.readByte() & 0xFF) << 24) | ((this.readByte() & 0xFF) << 16) | ((this.readByte() & 0xFF) << 8) | (this.readByte() & 0xFF);
	}

	@Override
	public double readDouble() {
		return Double.longBitsToDouble(this.readLong());
	}

	@Override
	public long readLong() {
		return (((long) this.readByte() & 0xFF) << 56) | (((long) this.readByte() & 0xFF) << 48) | (((long) this.readByte() & 0xFF) << 40) | (((long) this.readByte() & 0xFF) << 32) | (((long) this.readByte() & 0xFF) << 24) | (((long) this.readByte() & 0xFF) << 16) | (((long) this.readByte() & 0xFF) << 8) | ((long) this.readByte() & 0xFF);
	}

	FilePersistentReader readReader() {
		byte[] data = this.readByteArray();
		return new FilePersistentReader(data);
	}
}
