package com.khopan.hackontrol.persistent;

public interface IPersistentWriter {
	void writeBoolean(boolean value);
	void writeByte(byte value);
	void writeChar(char value);
	void writeShort(short value);
	void writeFloat(float value);
	void writeInt(int value);
	void writeDouble(double value);
	void writeLong(long value);

	default void writeBooleanArray(boolean[] array) {
		if(array == null) {
			this.writeInt(-1);
			return;
		}

		this.writeInt(array.length);

		for(int i = 0; i < array.length; i++) {
			this.writeBoolean(array[i]);
		}
	}

	default void writeByteArray(byte[] array) {
		if(array == null) {
			this.writeInt(-1);
			return;
		}

		this.writeInt(array.length);

		for(int i = 0; i < array.length; i++) {
			this.writeByte(array[i]);
		}
	}

	default void writeCharArray(char[] array) {
		if(array == null) {
			this.writeInt(-1);
			return;
		}

		this.writeInt(array.length);

		for(int i = 0; i < array.length; i++) {
			this.writeChar(array[i]);
		}
	}

	default void writeShortArray(short[] array) {
		if(array == null) {
			this.writeInt(-1);
			return;
		}

		this.writeInt(array.length);

		for(int i = 0; i < array.length; i++) {
			this.writeShort(array[i]);
		}
	}

	default void writeFloatArray(float[] array) {
		if(array == null) {
			this.writeInt(-1);
			return;
		}

		this.writeInt(array.length);

		for(int i = 0; i < array.length; i++) {
			this.writeFloat(array[i]);
		}
	}

	default void writeIntArray(int[] array) {
		if(array == null) {
			this.writeInt(-1);
			return;
		}

		this.writeInt(array.length);

		for(int i = 0; i < array.length; i++) {
			this.writeInt(array[i]);
		}
	}

	default void writeDoubleArray(double[] array) {
		if(array == null) {
			this.writeInt(-1);
			return;
		}

		this.writeInt(array.length);

		for(int i = 0; i < array.length; i++) {
			this.writeDouble(array[i]);
		}
	}

	default void writeLongArray(long[] array) {
		if(array == null) {
			this.writeInt(-1);
			return;
		}

		this.writeInt(array.length);

		for(int i = 0; i < array.length; i++) {
			this.writeLong(array[i]);
		}
	}

	default void writeString(String text) {
		this.writeCharArray(text == null ? null : text.toCharArray());
	}
}
