package com.khopan.hackontrol.registry;

public interface BiRegistrable<T, U> {
	public static <T, U> BiRegistrable<T, U> create() {
		return new BiRegistrable<>() {};
	}
}
