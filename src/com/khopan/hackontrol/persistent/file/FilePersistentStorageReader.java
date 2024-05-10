package com.khopan.hackontrol.persistent.file;

import java.lang.reflect.Constructor;
import java.util.Map;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.persistent.IPersistent;

class FilePersistentStorageReader {
	FilePersistentStorageReader(byte[] data, Map<String, IPersistent> map) {
		FilePersistentReader reader = new FilePersistentReader(data);
		int pairs = reader.readInt();

		for(int i = 0; i < pairs; i++) {
			try {
				this.readKeyValuePair(reader, map);
			} catch(Throwable Errors) {
				Hackontrol.LOGGER.info("Exception while reading persistent storage", Errors);
			}
		}
	}

	private void readKeyValuePair(FilePersistentReader reader, Map<String, IPersistent> map) {
		String key = reader.readString();
		String className = reader.readString();
		Class<?> valueClass;

		try {
			valueClass = Class.forName(className);
		} catch(ClassNotFoundException Exception) {
			Hackontrol.LOGGER.warn("Value class '{}' is not found for key '{}'", className, key);
			return;
		}

		if(!IPersistent.class.isAssignableFrom(valueClass)) {
			Hackontrol.LOGGER.warn("Value class '{}' is not a persistent object", className);
			return;
		}

		Constructor<?> constructor;

		try {
			constructor = valueClass.getDeclaredConstructor();
		} catch(NoSuchMethodException Exception) {
			Hackontrol.LOGGER.warn("Constructor not found for value class '{}', an IPersistent object must declare an empty constructor", className);
			return;
		}

		constructor.setAccessible(true);
		IPersistent persistent;

		try {
			persistent = (IPersistent) constructor.newInstance();
		} catch(Throwable Errors) {
			Hackontrol.LOGGER.warn("Error while constructing a persistent object", Errors);
			return;
		}

		persistent.read(reader.readReader());
		map.put(key, persistent);
	}
}
