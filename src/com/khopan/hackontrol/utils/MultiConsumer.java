package com.khopan.hackontrol.utils;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

public class MultiConsumer<T> implements Consumer<T> {
	private final List<Consumer<T>> list;

	public MultiConsumer() {
		this.list = new ArrayList<>();
	}

	@Override
	public void accept(T type) {
		for(int i = 0; i < this.list.size(); i++) {
			Consumer<T> consumer = this.list.get(i);

			if(consumer != null) {
				consumer.accept(type);
			}
		}
	}

	public void add(Consumer<T> consumer) {
		if(this.equals(consumer)) {
			throw new IllegalArgumentException("Cannot add itself to itself");
		}

		this.list.add(consumer);
	}

	public void addAll(List<Consumer<T>> list) {
		for(int i = 0; i < list.size(); i++) {
			this.add(list.get(i));
		}
	}
}
