package com.khopan.hackontrol.permission;

public enum Permission {
	BULLET(1181835151974596669L, 0x2ECC71, "Bullet"),
	SEA(1181835692570054686L, 0x3498DB, "Sea"),
	SPARK(1181836628201852928L, 0xF1C40F, "Spark"),
	NOODLE(1181837184450428949L, 0xE74C3C, "Noodle"),
	POPCORN(1181837601074855987L, 0x9B59B6, "Popcorn");

	private final long identifier;
	private final int color;
	private final String name;

	Permission(long identifier, int color, String name) {
		this.identifier = identifier;
		this.color = color;
		this.name = name;
	}

	public long getIdentifier() {
		return this.identifier;
	}

	public int getColor() {
		return this.color;
	}

	public String getName() {
		return this.name;
	}
}
