package com.khopan.hackontrol.registry;

import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;

public class RegistryType<T, U> {
	public final Class<?> registryHolderClass;

	private final Map<T, U> map;

	private RegistryType(Class<?> registryHolderClass) {
		this.registryHolderClass = registryHolderClass;
		this.map = new LinkedHashMap<>();
	}

	public void register(T identifier, U value) {
		if(identifier == null) {
			return;
		}

		if(this.map.containsKey(identifier)) {
			throw new IllegalArgumentException("Duplicate registry entry");
		}

		this.map.put(identifier, value);
	}

	public U get(T identifier) {
		return this.map.get(identifier);
	}

	public Map<T, U> map() {
		return Collections.unmodifiableMap(this.map);
	}

	@SuppressWarnings({ "rawtypes", "unchecked" })
	public static <T, U> RegistryType<T, U> create(Class<?> registryHolderClass) {
		if(registryHolderClass == null) {
			return null;
		}

		return new RegistryType(registryHolderClass);
	}
}
