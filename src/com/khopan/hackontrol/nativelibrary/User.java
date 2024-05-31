package com.khopan.hackontrol.nativelibrary;

public class User {
	private User() {}

	public static native float getMasterVolume();
	public static native boolean isMute();
	public static native void setMasterVolume(float volume);
	public static native void setMute(boolean mute);
	public static native int showMessageBox(String title, String content, int flags);
}
