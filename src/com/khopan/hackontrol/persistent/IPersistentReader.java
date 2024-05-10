package com.khopan.hackontrol.persistent;

public interface IPersistentReader {
	boolean readBoolean();
	byte readByte();
	char readChar();
	short readShort();
	float readFloat();
	int readInt();
	double readDouble();
	long readLong();

	default boolean[] readBooleanArray() {
		int length = this.readInt();

		if(length < 0) {
			return null;
		}

		boolean[] array = new boolean[length];

		for(int i = 0; i < length; i++) {
			array[i] = this.readBoolean();
		}

		return array;
	}

	default byte[] readByteArray() {
		int length = this.readInt();

		if(length < 0) {
			return null;
		}

		byte[] array = new byte[length];

		for(int i = 0; i < length; i++) {
			array[i] = this.readByte();
		}

		return array;
	}

	default char[] readCharArray() {
		int length = this.readInt();

		if(length < 0) {
			return null;
		}

		char[] array = new char[length];

		for(int i = 0; i < length; i++) {
			array[i] = this.readChar();
		}

		return array;
	}

	default short[] readShortArray() {
		int length = this.readInt();

		if(length < 0) {
			return null;
		}

		short[] array = new short[length];

		for(int i = 0; i < length; i++) {
			array[i] = this.readShort();
		}

		return array;
	}

	default float[] readFloatArray() {
		int length = this.readInt();

		if(length < 0) {
			return null;
		}

		float[] array = new float[length];

		for(int i = 0; i < length; i++) {
			array[i] = this.readFloat();
		}

		return array;
	}

	default int[] readIntArray() {
		int length = this.readInt();

		if(length < 0) {
			return null;
		}

		int[] array = new int[length];

		for(int i = 0; i < length; i++) {
			array[i] = this.readInt();
		}

		return array;
	}

	default double[] readDoubleArray() {
		int length = this.readInt();

		if(length < 0) {
			return null;
		}

		double[] array = new double[length];

		for(int i = 0; i < length; i++) {
			array[i] = this.readDouble();
		}

		return array;
	}

	default long[] readLongArray() {
		int length = this.readInt();

		if(length < 0) {
			return null;
		}

		long[] array = new long[length];

		for(int i = 0; i < length; i++) {
			array[i] = this.readLong();
		}

		return array;
	}

	default String readString() {
		char[] array = this.readCharArray();

		if(array == null) {
			return null;
		}

		return new String(array);
	}
}
