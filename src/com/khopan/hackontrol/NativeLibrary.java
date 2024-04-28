package com.khopan.hackontrol;

public class NativeLibrary {
	private NativeLibrary() {}

	static {
		System.load("D:\\GitHub Repository\\Hackontrol\\Hackontrol\\x64\\Release\\Native Library.dll");
	}

	public static native int sleep();
	public static native int hibernate();
	public static native int restart();
	public static native int shutdown();

	public static void load() {
		// Load the class
	}

	private static boolean log(int keyAction, int keyCode, int scanCode, int flags, int time) {
		System.out.println("Action: " + keyAction + " Code: " + keyCode + " Scan: " + scanCode + " Flags: " + flags + " Time: " + time);
		return false;
	}
}
