package com.khopan.hackontrol;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class KeyboardHandler {
	private KeyboardHandler() {}

	public static final List<KeyEntry> KEYSTROKE_LIST = new ArrayList<>();
	public static final Map<Integer, Boolean> KEYSTROKE_MAP = new HashMap<>();

	public static boolean Block = false;
	public static boolean Enable = false;
	public static boolean Freeze = false;

	public static boolean log(int keyAction, int keyCode, int scanCode, int flags, int time, String keyName) {
		boolean block = KeyboardHandler.Block || KeyboardHandler.Freeze;

		if(!KeyboardHandler.Enable) {
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
		entry.keyName = keyName;
		Boolean value = KeyboardHandler.KEYSTROKE_MAP.get(scanCode);
		entry.fake = value == null ? false : value == entry.keyDown;
		KeyboardHandler.KEYSTROKE_MAP.put(scanCode, entry.keyDown);
		KeyboardHandler.KEYSTROKE_LIST.add(entry);
		return block;
	}

	public static class KeyEntry {
		public boolean keyDown;
		public boolean systemKey;
		public int keyCode;
		public int scanCode;
		public String keyName;
		public boolean fake;
	}
}
