package com.khopan.hackontrol.registry.implementation;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.registry.RegistryType;

public class StrictClassValueOnlyRegistryImplementation<V> implements Registry {
	private final RegistryType<?, ?> allowedType;
	private final List<V> list;

	private StrictClassValueOnlyRegistryImplementation(RegistryType<?, ?> allowedType) {
		this.allowedType = allowedType;
		this.list = new ArrayList<>();
	}

	@Override
	public <T, U> void register(RegistryType<T, U> type, T identifier, U value) {
		this.register(type, value);
	}

	@SuppressWarnings("unchecked")
	@Override
	public <U> void register(RegistryType<?, U> type, U value) {
		if(!this.allowedType.equals(type)) {
			throw new IllegalArgumentException("Invalid registry type");
		}

		if(value == null) {
			return;
		}

		if(!(value instanceof Class)) {
			throw new IllegalArgumentException("Invalid value type");
		}

		Class<V> valueAsClass = (Class<V>) value;

		try {
			V valueInstance = valueAsClass.getDeclaredConstructor().newInstance();
			this.list.add(valueInstance);
		} catch(Throwable Errors) {
			throw new RuntimeException(Errors);
		}
	}

	public List<V> getList() {
		return Collections.unmodifiableList(this.list);
	}

	public static <V> StrictClassValueOnlyRegistryImplementation<V> create(RegistryType<?, ?> allowedType, Class<V> valueType) {
		return new StrictClassValueOnlyRegistryImplementation<V>(allowedType);
	}
}
