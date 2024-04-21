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

	private class Registration {
		private HackontrolChannel channel;
		private RegistryType<?, ?> type;
		private Object identifier;
		private Object value;
	}
}
