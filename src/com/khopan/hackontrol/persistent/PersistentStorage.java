package com.khopan.hackontrol.persistent;

public interface PersistentStorage {
	void save(String key, IPersistent persistent);
	IPersistent load(String key);
}
