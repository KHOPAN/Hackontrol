package com.khopan.logger.hackontrol;

import java.io.OutputStream;
import java.io.PrintStream;
import java.util.Locale;

import org.slf4j.Logger;

public class HackontrolLoggerInternal {
	private HackontrolLoggerInternal() {}

	private static final String STANDARD_OUTPUT = "Standard Output";
	private static final String STANDARD_ERROR = "Standard Error";

	static PrintStream out;
	static PrintStream err;

	static void staticInitialize() {
		HackontrolLoggerInternal.out = System.out;
		HackontrolLoggerInternal.err = System.err;
		System.setOut(new PrintStreamRedirector(HackontrolLoggerInternal.out, false, new HackontrolLogger(HackontrolLoggerInternal.STANDARD_OUTPUT)));
		System.setErr(new PrintStreamRedirector(HackontrolLoggerInternal.err, true, new HackontrolLogger(HackontrolLoggerInternal.STANDARD_ERROR)));
	}

	private static class PrintStreamRedirector extends PrintStream {
		private final boolean error;
		private final Logger logger;

		public PrintStreamRedirector(OutputStream output, boolean error, Logger logger) {
			super(output);
			this.error = error;
			this.logger = logger;
		}

		@Override
		public void print(boolean value) {
			this.log(Boolean.toString(value));
		}

		@Override
		public void print(char character) {
			this.log(String.valueOf(character));
		}

		@Override
		public void print(char[] text) {
			this.log(new String(text));
		}

		@Override
		public void print(double value) {
			this.log(Double.toString(value));
		}

		@Override
		public void print(float value) {
			this.log(Float.toString(value));
		}

		@Override
		public void print(int value) {
			this.log(Integer.toString(value));
		}

		@Override
		public void print(long value) {
			this.log(Long.toString(value));
		}

		@Override
		public void print(Object instance) {
			this.log(String.valueOf(instance));
		}

		@Override
		public void print(String text) {
			this.log(String.valueOf(text));
		}

		@Override
		public PrintStream printf(Locale locale, String format, Object... args) {
			this.log(String.format(locale, format, args));
			return this;
		}

		@Override
		public PrintStream printf(String format, Object... args) {
			this.log(String.format(format, args));
			return this;
		}

		@Override
		public void println() {
			this.log("");
		}

		@Override
		public void println(boolean value) {
			this.log(Boolean.toString(value));
		}

		@Override
		public void println(char character) {
			this.log(String.valueOf(character));
		}

		@Override
		public void println(char[] text) {
			this.log(new String(text));
		}

		@Override
		public void println(double value) {
			this.log(Double.toString(value));
		}

		@Override
		public void println(float value) {
			this.log(Float.toString(value));
		}

		@Override
		public void println(int value) {
			this.log(Integer.toString(value));
		}

		@Override
		public void println(long value) {
			this.log(Long.toString(value));
		}

		@Override
		public void println(Object instance) {
			this.log(String.valueOf(instance));
		}

		@Override
		public void println(String text) {
			this.log(String.valueOf(text));
		}

		@Override
		public PrintStream format(Locale locale, String format, Object... args) {
			this.log(String.format(locale, format, args));
			return this;
		}

		@Override
		public PrintStream format(String format, Object... args) {
			this.log(String.format(format, args));
			return this;
		}

		private void log(String text) {
			if(this.error) {
				this.logger.error(text);
			} else {
				this.logger.info(text);
			}
		}
	}
}
