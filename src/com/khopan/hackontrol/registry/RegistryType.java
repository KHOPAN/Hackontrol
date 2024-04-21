package com.khopan.hackontrol.registry;

public class RegistryType<T, U> {
	private RegistryType() {}

	@SuppressWarnings({ "rawtypes", "unchecked" })
	public static <T, U> RegistryType<T, U> create() {
		return new RegistryType();
	}
}
