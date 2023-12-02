package com.khopan.hackontrol.updater;

import java.io.OutputStream;
import java.io.PrintStream;
import java.util.prefs.Preferences;

public class Administrator {
	// https://stackoverflow.com/a/23538961/17136195
	public static boolean hasAdministratorPrivileges() {
		Preferences preferences = Preferences.systemRoot();

		synchronized(System.err) {
			System.setErr(new PrintStream(new OutputStream() {
				@Override
				public void write(int data) {

				}
			}));

			try {
				preferences.put("foo", "bar"); // SecurityException on Windows
				preferences.remove("foo");
				preferences.flush(); // BackingStoreException on Linux
				return true;
			} catch(Exception Exception) {
				return false;
			} finally {
				System.setErr(System.err);
			}
		}
	}
}
