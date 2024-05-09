package com.khopan.hackontrol.registry.implementation;

import com.khopan.hackontrol.module.Module;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.registry.RegistryType;

public class FilteredTypeRegistry implements Registry {
	private final Module module;
	private final RegistryType<?, ?> allowedType;

	private FilteredTypeRegistry(Module module, RegistryType<?, ?> allowedType) {
		this.module = module;
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

		type.register(this.module, identifier, value);
	}

	@Override
	public <U> void register(RegistryType<?, U> type, U value) {
		this.register(type, null, value);
	}

	public static FilteredTypeRegistry of(Module module, RegistryType<?, ?> allowedType) {
		return new FilteredTypeRegistry(module, allowedType);
	}
}
