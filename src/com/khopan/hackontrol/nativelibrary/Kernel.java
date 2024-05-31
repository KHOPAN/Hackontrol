package com.khopan.hackontrol.nativelibrary;

public class Kernel {
	private Kernel() {}

	public static native void sleep();
	public static native void hibernate();
	public static native void shutdown();
	public static native void restart();
}
