package com.khopan.hackontrol.persistent;

public interface IPersistent {
	void write(IPersistentWriter writer);
	void read(IPersistentReader reader);
}
