package com.khopan.hackontrol.registry;

public interface Registry {
	<T, U> void register(RegistryType<T, U> type, T identifier, U value);
	<U> void register(RegistryType<?, U> type, U value);
}
