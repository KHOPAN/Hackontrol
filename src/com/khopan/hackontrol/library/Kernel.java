package com.khopan.hackontrol.library;

import java.util.function.Consumer;

import com.khopan.hackontrol.ProcessEntry;

public class Kernel {
	private Kernel() {}

	public static native void sleep();
	public static native void hibernate();
	public static native void restart();
	public static native void shutdown();
	public static native void setProcessCritical(boolean critical);
	public static native void setFreeze(boolean freeze, byte[] image);
	public static native ProcessEntry[] getProcessList();
	public static native int getCurrentProcessIdentifier();
	public static native void terminateProcess(int processIdentifier);
	public static native void initiateRestart(boolean update);
	public static native void shellExecute(String command, Consumer<String> callback);
	public static native void connectHRSPServer(String host, int port, Consumer<String> callback);
}
