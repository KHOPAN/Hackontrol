package com.khopan.hackontrol.registry;

import java.util.ArrayList;
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
		List<TypeEntry<T, U>> list = this.map.get(channel);

		if(list == null) {
			return new ArrayList<>();
		}

		return list;
	}

	public List<U> filter(HackontrolChannel channel, T identifier) {
		List<TypeEntry<T, U>> list = this.list(channel);
		List<U> result = new ArrayList<>();

		for(int i = 0; i< list.size(); i++) {
			TypeEntry<T, U> entry = list.get(i);
			T entryIdentifier = entry.getIdentifier();

			if(entryIdentifier == identifier || (entryIdentifier != null && entryIdentifier.equals(identifier))) {
				result.add(entry.getValue());
			}
		}

		return result;
	}

	@SuppressWarnings({ "rawtypes", "unchecked" })
	public static <T, U> RegistryType<T, U> create() {
		return new RegistryType();
	}
}
