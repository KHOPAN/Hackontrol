package com.khopan.hackontrol.registry.implementation;

import com.khopan.hackontrol.HackontrolChannel;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.registry.RegistryType;

public class RegistryImplementation implements Registry {
	private final HackontrolChannel channel;

	private RegistryImplementation(HackontrolChannel channel) {
		this.channel = channel;
	}

	@Override
	public <T, U> void register(RegistryType<T, U> type, T identifier, U value) {
		if(type == null) {
			throw new NullPointerException("Type cannot be null");
		}

		type.register(this.channel, identifier, value);
	}

	@Override
	public <U> void register(RegistryType<?, U> type, U value) {
		this.register(type, null, value);
	}

	public static RegistryImplementation of(HackontrolChannel channel) {
		return new RegistryImplementation(channel);
	}
}
