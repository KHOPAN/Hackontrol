package com.khopan.hackontrol.registry;

public class TypeEntry<T, U> {
	private final T identifier;
	private final U value;

	public TypeEntry(T identifier, U value) {
		this.identifier = identifier;
		this.value = value;
	}

	public T getIdentifier() {
		return this.identifier;
	}

	public U getValue() {
		return this.value;
	}
}
