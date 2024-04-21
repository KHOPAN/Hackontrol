package com.khopan.hackontrol.registry;

@FunctionalInterface
public interface Registry {
	<T, U> void register(RegistryType<T, U> type, T identifier, U value);
}
