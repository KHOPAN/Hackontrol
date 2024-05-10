package com.khopan.hackontrol.persistent;

import com.khopan.hackontrol.persistent.reader.IPersistentReader;
import com.khopan.hackontrol.persistent.writer.IPersistentWriter;

public interface IPersistent {
	void write(IPersistentWriter writer);
	void read(IPersistentReader reader);
}
