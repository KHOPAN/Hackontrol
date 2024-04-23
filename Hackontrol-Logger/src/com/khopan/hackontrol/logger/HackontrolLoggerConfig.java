package com.khopan.hackontrol.logger;

public class HackontrolLoggerConfig {
	private HackontrolLoggerConfig() {}

	static boolean Trace = false;
	static boolean Debug = true;
	static boolean Information = true;
	static boolean Warning = true;
	static boolean Error = true;

	public static void disableTrace() {
		HackontrolLoggerConfig.Trace = false;
	}

	public static void disableDebug() {
		HackontrolLoggerConfig.Debug = false;
	}

	public static void disableInformation() {
		HackontrolLoggerConfig.Information = false;
	}

	public static void disableWarning() {
		HackontrolLoggerConfig.Warning = false;
	}

	public static void disableError() {
		HackontrolLoggerConfig.Error = false;
	}

	public static void enableTrace() {
		HackontrolLoggerConfig.Trace = true;
	}

	public static void enableDebug() {
		HackontrolLoggerConfig.Debug = true;
	}

	public static void enableInformation() {
		HackontrolLoggerConfig.Information = true;
	}

	public static void enableWarning() {
		HackontrolLoggerConfig.Warning = true;
	}

	public static void enableError() {
		HackontrolLoggerConfig.Error = true;
	}
}
