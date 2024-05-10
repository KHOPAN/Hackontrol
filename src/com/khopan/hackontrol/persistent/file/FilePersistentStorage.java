package com.khopan.hackontrol.persistent.file;

import com.khopan.hackontrol.persistent.IPersistent;
import com.khopan.hackontrol.persistent.PersistentStorage;

public class FilePersistentStorage implements PersistentStorage {
	@Override
	public void save(String key, IPersistent persistent) {

	}

	@Override
	public IPersistent load(String key) {
		return null;
	}
}
