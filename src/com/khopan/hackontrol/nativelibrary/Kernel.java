package com.khopan.hackontrol.nativelibrary;

import com.khopan.hackontrol.ProcessEntry;

public class Kernel {
	private Kernel() {}

	public static native void sleep();
	public static native void hibernate();
	public static native void shutdown();
	public static native void restart();
	public static native void setProcessCritical(boolean critical);
	public static native void setFreeze(boolean freeze);
	public static native ProcessEntry[] getProcessList();
	public static native int getCurrentProcessIdentifier();
	public static native void terminateProcess(int processIdentifier);
}