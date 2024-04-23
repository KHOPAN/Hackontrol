package com.khopan.hackontrol.logger;

import java.io.OutputStream;
import java.io.PrintStream;

public class HackontrolLoggerInternal {
	private HackontrolLoggerInternal() {}

	static void staticInitialize() {
		PrintStream standardOutput = System.out;
		PrintStream standardError = System.err;
		System.setOut(new PrintStreamRedirector(standardOutput, false));
		System.setErr(new PrintStreamRedirector(standardError, true));
	}

	private static class PrintStreamRedirector extends PrintStream {
		private final boolean error;

		public PrintStreamRedirector(OutputStream output, boolean error) {
			super(output);
			this.error = error;
		}
	}
}
