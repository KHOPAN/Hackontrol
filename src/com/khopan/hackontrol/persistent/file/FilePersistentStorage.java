package com.khopan.hackontrol.persistent.file;

import java.io.File;
import java.io.FileInputStream;
import java.util.HashMap;
import java.util.Map;

import com.khopan.hackontrol.persistent.IPersistent;
import com.khopan.hackontrol.persistent.PersistentStorage;

public class FilePersistentStorage implements PersistentStorage {
	final Map<String, IPersistent> map;
	final File file;

	volatile boolean pendingWrite;
	volatile boolean writing;

	public FilePersistentStorage(File file) {
		this.map = new HashMap<>();
		this.file = file;
		FilePersistentStorageWriterWorker.start(this);

		try {
			boolean exists = this.file.exists();

			if(exists && this.file.isFile()) {
				FileInputStream stream = new FileInputStream(this.file);
				byte[] data = stream.readAllBytes();
				stream.close();

				if(data != null && data.length > 0) {
					new FilePersistentStorageReader(data, this.map);
				}

				return;
			}

			if(exists) {
				this.file.delete();
			}

			File parent = this.file.getParentFile();

			if(!parent.exists()) {
				parent.mkdirs();
			}

			this.file.createNewFile();
		} catch(Throwable Errors) {
			throw new RuntimeException("Error while reading the persistent storage", Errors);
		}
	}

	@Override
	public <T extends IPersistent> void save(String key, T persistent, Class<T> persistentClass) {
		if(key == null) {
			throw new NullPointerException("Persistent key cannot be null");
		}

		if(persistentClass == null) {
			throw new NullPointerException("Persistent class cannot be null");
		}

		if(persistent == null) {
			persistent = this.createPersistent(persistentClass);
		}

		if(this.writing) {
			while(this.writing);
		}

		this.map.put(key, persistent);
		this.pendingWrite = true;
	}

	@SuppressWarnings("unchecked")
	@Override
	public <T extends IPersistent> T load(String key, Class<T> persistentClass) {
		if(key == null) {
			throw new NullPointerException("Persistent key cannot be null");
		}

		if(persistentClass == null) {
			throw new NullPointerException("Persistent class cannot be null");
		}

		T persistent = (T) this.map.get(key);

		if(persistent == null) {
			persistent = this.createPersistent(persistentClass);
		}

		return persistent;
	}

	private <T> T createPersistent(Class<T> persistentClass) {
		try {
			return (T) persistentClass.getDeclaredConstructor().newInstance();
		} catch(Throwable Errors) {
			throw new RuntimeException("Error while creating persistent instance", Errors);
		}
	}
}
