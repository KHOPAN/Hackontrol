package com.khopan.hackontrol.library;

public class Information {
	private Information() {}

	public static native String getToken();
	public static native String getUserName();
	public static native String getMachineName();
	public static native boolean isEnabledUIAccess();
}
