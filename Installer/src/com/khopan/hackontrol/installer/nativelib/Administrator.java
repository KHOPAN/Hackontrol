package com.khopan.hackontrol.installer.nativelib;

public class Administrator {
	static {
		Native.load();
	}

	public static native boolean hasAdministratorPrivileges();
}
