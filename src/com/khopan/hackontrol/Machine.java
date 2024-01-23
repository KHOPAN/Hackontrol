package com.khopan.hackontrol;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.Random;

public class Machine {
	private Machine() {}

	public static String getIdentifier() {
		ProcessBuilder processBuilder = new ProcessBuilder("wmic", "csproduct", "get", "uuid");
		processBuilder.redirectErrorStream(true);
		Process process;

		try {
			process = processBuilder.start();
		} catch(Throwable ignored) {
			return Machine.getRandomMachineId();
		}

		InputStream stream = process.getInputStream();
		BufferedReader reader = new BufferedReader(new InputStreamReader(stream));
		StringBuilder stringBuilder = new StringBuilder();
		String line;

		try {
			while((line = reader.readLine()) != null) {
				stringBuilder.append(line);
			}
		} catch(Throwable ignore) {
			return Machine.getRandomMachineId();
		}

		String result = stringBuilder.toString();
		int length = result.length();

		if(length < 5) {
			return Machine.getRandomMachineId();
		}

		StringBuilder idBuilder = new StringBuilder();

		for(int i = 4; i < length; i++) {
			char character = result.charAt(i);

			if(!Machine.isHex(character)) {
				continue;
			}

			idBuilder.append(character);
		}

		MessageDigest digest;

		try {
			digest = MessageDigest.getInstance("SHA-1");
		} catch(Throwable ignored) {
			return Machine.getRandomMachineId();
		}

		String identifierText = idBuilder.toString();
		byte[] identifierBytes = identifierText.getBytes(StandardCharsets.UTF_8);
		byte[] hashedBytes = digest.digest(identifierBytes);
		StringBuilder hashBuilder = new StringBuilder();

		for(int i = 0; i < hashedBytes.length; i++) {
			hashBuilder.append(String.format("%02x", hashedBytes[i]).toLowerCase());
		}

		return hashBuilder.toString();
	}

	private static String getRandomMachineId() {
		Random random = new Random(System.nanoTime() - System.currentTimeMillis());
		char[] hexMap = new char[] {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
		StringBuilder builder = new StringBuilder();

		for(int i = 0; i < 40; i++) {
			double value = random.nextDouble();
			int index = Math.min(Math.max((int) Math.round(value * 15.0d), 0), 15);
			builder.append(hexMap[index]);
		}

		return builder.toString();
	}

	private static boolean isHex(char character) {
		switch(character) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			return true;
		}

		return false;
	}
}
