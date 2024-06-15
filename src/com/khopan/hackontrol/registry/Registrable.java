package com.khopan.hackontrol.registry;

public interface Registrable<T> {
	public static <T> Registrable<T> create() {
		return new Registrable<>() {};
	}
}
