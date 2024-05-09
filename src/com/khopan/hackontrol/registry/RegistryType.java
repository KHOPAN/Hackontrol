package com.khopan.hackontrol.registry;

import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import com.khopan.hackontrol.HackontrolChannel;

public class RegistryType<T, U> {
	private final Map<HackontrolChannel, List<TypeEntry<T, U>>> map;

	private RegistryType() {
		this.map = new LinkedHashMap<>();
	}

	public void register(HackontrolChannel channel, T identifier, U value) {
		if(!this.map.containsKey(channel)) {
			this.map.put(channel, new ArrayList<>());
		}

		this.map.get(channel).add(new TypeEntry<>(identifier, value));
	}

	public List<TypeEntry<T, U>> list(HackontrolChannel channel) {
		return Collections.unmodifiableList(this.map.get(channel));
	}

	@SuppressWarnings({ "rawtypes", "unchecked" })
	public static <T, U> RegistryType<T, U> create() {
		return new RegistryType();
	}
}
