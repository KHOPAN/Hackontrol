package com.khopan.hackontrol.permission;

public enum Permission {
	BULLET(1181835151974596669L, 0x2ECC71),
	SEA(1181835692570054686L, 0x3498DB),
	SPARK(1181836628201852928L, 0xF1C40F),
	NOODLE(1181837184450428949L, 0xE74C3C),
	POPCORN(1181837601074855987L, 0x9B59B6);

	private final long identifier;
	private final int color;

	Permission(long identifier, int color) {
		this.identifier = identifier;
		this.color = color;
	}

	public long getIdentifier() {
		return this.identifier;
	}

	public int getColor() {
		return this.color;
	}
}
