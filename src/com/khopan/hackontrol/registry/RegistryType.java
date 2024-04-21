package com.khopan.hackontrol.registry;

public class RegistryType<T, U> {
	public final Class<?> registryHolderClass;

	private RegistryType(Class<?> registryHolderClass) {
		this.registryHolderClass = registryHolderClass;
	}

	@SuppressWarnings({ "rawtypes", "unchecked" })
	public static <T, U> RegistryType<T, U> create(Class<?> registryHolderClass) {
		if(registryHolderClass == null) {
			return null;
		}

		return new RegistryType(registryHolderClass);
	}
}
