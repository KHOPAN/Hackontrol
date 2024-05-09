package com.khopan.hackontrol.registry;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import com.khopan.hackontrol.module.Module;

public class RegistryType<T, U> {
	private final Map<Module, List<TypeEntry<T, U>>> map;

	private RegistryType() {
		this.map = new LinkedHashMap<>();
	}

	public void register(Module module, T identifier, U value) {
		if(!this.map.containsKey(module)) {
			this.map.put(module, new ArrayList<>());
		}

		this.map.get(module).add(new TypeEntry<>(identifier, value));
	}

	public List<TypeEntry<T, U>> list(Module module) {
		List<TypeEntry<T, U>> list = this.map.get(module);

		if(list == null) {
			return new ArrayList<>();
		}

		return list;
	}

	public List<U> filter(Module module, T identifier) {
		List<TypeEntry<T, U>> list = this.list(module);
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
