package com.khopan.logger.hackontrol;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.ILoggerFactory;
import org.slf4j.Logger;

public class HackontrolLoggerFactory implements ILoggerFactory {
	private final ConcurrentMap<String, Logger> map;

	public HackontrolLoggerFactory() {
		this.map = new ConcurrentHashMap<>();
	}

	@Override
	public Logger getLogger(String name) {
		Logger logger = this.map.get(name);

		if(logger != null) {
			return logger;
		}

		Logger newInstance = new HackontrolLogger(name);
		Logger oldInstance = this.map.putIfAbsent(name, newInstance);
		return oldInstance == null ? newInstance : oldInstance;
	}
}
