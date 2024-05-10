package com.khopan.hackontrol.persistent.builtin;

import com.khopan.hackontrol.persistent.IPersistent;
import com.khopan.hackontrol.persistent.IPersistentReader;
import com.khopan.hackontrol.persistent.IPersistentWriter;

public class StringPresistent implements IPersistent {
	public String value;

	@Override
	public void write(IPersistentWriter writer) {
		writer.writeString(this.value);
	}

	@Override
	public void read(IPersistentReader reader) {
		this.value = reader.readString();
	}
}
