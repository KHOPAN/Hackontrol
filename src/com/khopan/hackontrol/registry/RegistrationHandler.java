package com.khopan.hackontrol.registry;

import java.util.ArrayList;
import java.util.List;

import com.khopan.hackontrol.HackontrolChannel;

public class RegistrationHandler {
	private final List<Registration> list;

	public RegistrationHandler() {
		this.list = new ArrayList<>();
	}

	public Registry createRegistry(HackontrolChannel channel) {
		if(channel == null) {
			return null;
		}

		return new RegistrationHandlerRegistryImplementation(channel);
	}

	@SuppressWarnings("unchecked")
	public <T, U> List<RegistrationTypeEntry<T, U>> filterType(RegistryType<T, U> type) {
		if(type == null) {
			return null;
		}

		List<RegistrationTypeEntry<T, U>> result = new ArrayList<>();

		for(int i = 0; i < this.list.size(); i++) {
			Registration registration = this.list.get(i);

			if(!type.equals(registration.type)) {
				continue;
			}

			RegistrationTypeEntry<T, U> entry = new RegistrationTypeEntry<>();
			entry.channel = registration.channel;
			entry.identifier = (T) registration.identifier;
			entry.value = (U) registration.value;
			result.add(entry);
		}

		return result;
	}

	private class RegistrationHandlerRegistryImplementation implements Registry {
		private final HackontrolChannel channel;

		private RegistrationHandlerRegistryImplementation(HackontrolChannel channel) {
			this.channel = channel;
		}

		@Override
		public <T, U> void register(RegistryType<T, U> type, T identifier, U value) {
			Registration reigstration = new Registration();
			reigstration.channel = this.channel;
			reigstration.type = type;
			reigstration.identifier = identifier;
			reigstration.value = value;
			RegistrationHandler.this.list.add(reigstration);
		}

		@Override
		public <U> void register(RegistryType<?, U> type, U value) {
			this.register(type, null, value);
		}
	}

	private static boolean compare(Object x, Object y) {
		if(x == y) {
			return true;
		}

		return x != null && x.equals(y);
	}

	private class Registration {
		private HackontrolChannel channel;
		private RegistryType<?, ?> type;
		private Object identifier;
		private Object value;
	}

	public static class RegistrationTypeEntry<T, U> {
		public HackontrolChannel channel;
		public T identifier;
		public U value;

		public static <T, U> U filter(List<RegistrationTypeEntry<T, U>> list, HackontrolChannel channel, T identifier) {
			if(list == null || channel == null) {
				return null;
			}

			for(int i = 0; i < list.size(); i++) {
				RegistrationTypeEntry<T, U> entry = list.get(i);

				if(!channel.equals(entry.channel)) {
					continue;
				}

				if(!RegistrationHandler.compare(identifier, entry.identifier)) {
					continue;
				}

				return entry.value;
			}

			return null;
		}
	}
}
