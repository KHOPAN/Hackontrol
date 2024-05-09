package com.khopan.hackontrol.registry.implementation;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.registry.RegistryType;

public class FilteredTypeRegistry implements Registry {
	private final HackontrolChannel channel;
	private final RegistryType<?, ?> allowedType;

	private FilteredTypeRegistry(HackontrolChannel channel, RegistryType<?, ?> allowedType) {
		this.channel = channel;
		this.allowedType = allowedType;
	}

	@Override
	public <T, U> void register(RegistryType<T, U> type, T identifier, U value) {
		if(type == null) {
			throw new NullPointerException("Type cannot be null");
		}

		if(!this.allowedType.equals(type)) {
			throw new IllegalArgumentException("Registry type '" + type.getClass().getName() + "' is not allowed");
		}

		type.register(this.channel, identifier, value);
	}

	@Override
	public <U> void register(RegistryType<?, U> type, U value) {
		this.register(type, null, value);
	}

	public static FilteredTypeRegistry of(HackontrolChannel channel, RegistryType<?, ?> allowedType) {
		return new FilteredTypeRegistry(channel, allowedType);
	}
}
