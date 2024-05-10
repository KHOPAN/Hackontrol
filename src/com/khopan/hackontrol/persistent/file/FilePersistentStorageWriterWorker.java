package com.khopan.hackontrol.persistent.file;

import java.io.FileOutputStream;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.Set;

import com.khopan.hackontrol.Hackontrol;
import com.khopan.hackontrol.persistent.IPersistent;

class FilePersistentStorageWriterWorker implements Runnable {
	private final FilePersistentStorage storage;

	private FilePersistentStorageWriterWorker(FilePersistentStorage storage) {
		this.storage = storage;
	}

	@Override
	public void run() {
		while(true) {
			try {
				Thread.sleep(100);

				if(!this.storage.pendingWrite) {
					continue;
				}

				this.storage.writing = true;
				this.performWrite();
				this.storage.writing = false;
				this.storage.pendingWrite = false;
			} catch(Throwable Errors) {
				Hackontrol.LOGGER.warn("Exception while writing persistent storage", Errors);
			}
		}
	}

	private void performWrite() throws Throwable {
		FilePersistentWriter writer = new FilePersistentWriter();
		Set<Entry<String, IPersistent>> set = this.storage.map.entrySet();
		Iterator<Entry<String, IPersistent>> iterator = set.iterator();
		writer.writeInt(set.size());

		while(iterator.hasNext()) {
			Entry<String, IPersistent> entry = iterator.next();
			String key = entry.getKey();
			IPersistent value = entry.getValue();
			writer.writeString(key);
			writer.writeString(value.getClass().getName());
			FilePersistentWriter valueWriter = new FilePersistentWriter();
			value.write(valueWriter);
			writer.writeWriter(valueWriter);
		}

		byte[] data = writer.getData();
		FileOutputStream stream = new FileOutputStream(this.storage.file);
		stream.write(data);
		stream.close();
	}

	static void start(FilePersistentStorage storage) {
		FilePersistentStorageWriterWorker worker = new FilePersistentStorageWriterWorker(storage);
		Thread thread = new Thread(worker);
		thread.setName("Hackontrol File Persistent Writer");
		thread.setPriority(7);
		thread.start();
	}
}
