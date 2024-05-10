package com.khopan.hackontrol.persistent;

public interface PersistentStorage {
	<T extends IPersistent> void save(String key, T persistent, Class<T> persistentClass);
	<T extends IPersistent> T load(String key, Class<T> persistentClass);
}
