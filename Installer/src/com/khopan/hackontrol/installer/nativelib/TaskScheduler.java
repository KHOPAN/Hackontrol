package com.khopan.hackontrol.installer.nativelib;

public class TaskScheduler {
	static {
		Native.load();
	}

	public static native boolean registerTask(String taskName, String executable);
}
