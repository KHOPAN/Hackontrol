package com.khopan.hackontrol.registry;

import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.HackontrolChannel;

public class ClassRegistration {
	private ClassRegistration() {}

	public static <T, U> List<TypeEntry<T, U>> newInstance(HackontrolChannel channel, RegistryType<T, Class<? extends U>> registryType) {
		try {
			List<TypeEntry<T, Class<? extends U>>> list = registryType.list(channel);
			List<TypeEntry<T, U>> result = new ArrayList<>();

			for(int i = 0; i < list.size(); i++) {
				TypeEntry<T, Class<? extends U>> entry = list.get(i);
				result.add(new TypeEntry<>(entry.getIdentifier(), entry.getValue().getConstructor().newInstance()));
			}

			return result;
		} catch(Throwable Errors) {
			throw new RuntimeException(Errors);
		}
	}

	public static <T, U> List<U> list(HackontrolChannel channel, RegistryType<T, Class<? extends U>> registryType) {
		List<TypeEntry<T, U>> list = ClassRegistration.newInstance(channel, registryType);
		List<U> result = new ArrayList<>();

		for(int i = 0; i < list.size(); i++) {
			result.add(list.get(i).getValue());
		}

		return result;
	}
}
