package com.khopan.hackontrol.registry.implementation;

import com.khopan.hackontrol.module.Module;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.registry.RegistryType;

public class RegistryImplementation implements Registry {
	private final Module module;

	private RegistryImplementation(Module module) {
		this.module = module;
	}

	@Override
	public <T, U> void register(RegistryType<T, U> type, T identifier, U value) {
		if(type == null) {
			throw new NullPointerException("Type cannot be null");
		}

		type.register(this.module, identifier, value);
	}

	@Override
	public <U> void register(RegistryType<?, U> type, U value) {
		this.register(type, null, value);
	}

	public static RegistryImplementation of(Module module) {
		return new RegistryImplementation(module);
	}
}
