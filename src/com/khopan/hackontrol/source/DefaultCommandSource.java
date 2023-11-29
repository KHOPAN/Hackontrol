package com.khopan.hackontrol.source;

import java.util.function.Consumer;
import java.util.function.Supplier;

import net.dv8tion.jda.api.entities.channel.middleman.MessageChannel;

public class DefaultCommandSource implements CommandSource {
	private final String machineIdentifier;
	private final MessageChannel channel;
	private final Supplier<Boolean> selected;
	private final Consumer<Boolean> setSelected;

	public DefaultCommandSource(String machineIdentifier, MessageChannel channel, Supplier<Boolean> selected, Consumer<Boolean> setSelected) {
		this.machineIdentifier = machineIdentifier;
		this.channel = channel;
		this.selected = selected;
		this.setSelected = setSelected;
	}

	@Override
	public String getMachineId() {
		return this.machineIdentifier;
	}

	@Override
	public void sendMessage(String message) {
		this.channel.sendMessage(message).queue();
	}

	@Override
	public void sendCodeMessage(String message) {
		this.sendMessage('`' + message + '`');
	}

	@Override
	public void sendBoldCodeMessage(String message) {
		this.sendMessage("**`" + message + "`**");
	}

	@Override
	public boolean isSelected() {
		return this.selected.get();
	}

	@Override
	public void setSelected(boolean selected) {
		if(this.selected.get() == selected) {
			return;
		}

		this.setSelected.accept(selected);
		this.sendMessage('`' + this.machineIdentifier + "` was " + (selected ? "" : "un") + "selected!");
	}
}
