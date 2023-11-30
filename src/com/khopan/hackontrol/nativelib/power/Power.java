package com.khopan.hackontrol.nativelib.power;

import com.khopan.hackontrol.nativelib.Native;

public class Power {
	static {
		Native.load();
	}

	public static native int ExitWindowsEx(int uFlags, long dwReason);
}
