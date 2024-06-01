package com.khopan.hackontrol;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class NativeLibrary {
	private NativeLibrary() {}

	public static final List<KeyEntry> KEYSTROKE_LIST = new ArrayList<>();
	public static final Map<Integer, Boolean> KEYSTROKE_MAP = new HashMap<>();

	public static boolean Block = false;
	public static boolean Enable = false;
	public static boolean Freeze = false;

	private static boolean log(int keyAction, int keyCode, int scanCode, int flags, int time) {
		boolean block = NativeLibrary.Block || NativeLibrary.Freeze;

		if(!NativeLibrary.Enable) {
			return block;
		}

		KeyEntry entry = new KeyEntry();

		switch(keyAction) {
		case 0x0100:
			entry.keyDown = true;
			entry.systemKey = false;
			break;
		case 0x0101:
			entry.keyDown = false;
			entry.systemKey = false;
			break;
		case 0x0104:
			entry.keyDown = true;
			entry.systemKey = true;
			break;
		case 0x0105:
			entry.keyDown = false;
			entry.systemKey = true;
			break;
		}

		entry.keyCode = keyCode;
		entry.scanCode = scanCode;
		Boolean value = NativeLibrary.KEYSTROKE_MAP.get(scanCode);
		boolean previousState = value == null ? false : value;
		entry.fake = previousState == entry.keyDown;
		NativeLibrary.KEYSTROKE_MAP.put(scanCode, entry.keyDown);
		NativeLibrary.KEYSTROKE_LIST.add(entry);
		return block;
	}

	public static class KeyEntry {
		public boolean keyDown;
		public boolean systemKey;
		public int keyCode;
		public int scanCode;
		public boolean fake;
	}
}
