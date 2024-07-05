package com.khopan.hackontrol;

public class Win32Error extends Error {
	private static final long serialVersionUID = -4873748328320278492L;

	public Win32Error() {
		super();
	}

	public Win32Error(String message) {
		super(message);
	}

	public Win32Error(String message, Throwable cause) {
		super(message, cause);
	}

	public Win32Error(Throwable cause) {
		super(cause);
	}

	public Win32Error(String message, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
		super(message, cause, enableSuppression, writableStackTrace);
	}
}
