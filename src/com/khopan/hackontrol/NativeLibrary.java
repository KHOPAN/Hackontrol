package com.khopan.hackontrol;

public class NativeLibrary {
	private NativeLibrary() {}

	static {
		System.load("D:\\GitHub Repository\\Hackontrol\\Hackontrol\\x64\\Release\\Native Library.dll");
	}

	public static void load() {
		// Load the class
	}
}
